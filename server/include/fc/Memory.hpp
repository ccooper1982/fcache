#pragma once

#include <memory_resource>
#include <functional>
#include <plog/Log.h>
#include <format>

namespace fc
{
  /// Uses a pool resource backed by monotonic resource:
  ///   - when the pool requires memory it requests a buffer from the monotonic resource
  ///   - a mononotic resource only releases memory when the resource is destroyed
  ///   - but that's ok because the pool tracks unused memory (free list) within the buffer allocated by the monotonic
  ///   - when a value is erased, the free list is aware and can reuse it
  ///
  /// This has been influenced by Jason Turner's video:
  ///   https://www.youtube.com/watch?v=Zt0q3OEeuB0&list=PLs3KjaCtOwSYX3X0L36NgwK0pxZZavDSF&index=5&t=265
  /// Specifically from 6mins45.
  ///
  /// - The pool resource allocates an internal structure to track pools (often/always 528 bytes)
  /// - The pool resource contains multiple pools, each intended for different size requirements
  /// - Each of those pools contains multiple blocks (which are logically split into chunks)


  #ifdef FC_DEBUG
    // Doesn't directly allocate/deallocate, only logs then passes alloc/dealloc requests to upstream.
    class PrintResource : public std::pmr::memory_resource
    {
    public:
      PrintResource(std::string name, std::pmr::memory_resource* upstream, std::function<void(std::byte *, const std::size_t)> deallocCheck = nullptr)
          : m_name(std::move(name)),
            m_upstream(upstream),
            m_deallocCheck(deallocCheck)
      {
        assert(upstream);
      }


    private:
      std::string m_name;
      std::pmr::memory_resource * m_upstream;

      void * do_allocate(std::size_t bytes, std::size_t alignment) override
      {
        auto result = m_upstream->allocate(bytes, alignment);
        PLOGD << m_name << " ALLOC: Size: " << bytes << ", Align: " << alignment << ", Address: " << result;
        return result;
      }


      std::string format_destroyed_bytes(std::byte* p, const std::size_t size)
      {
        std::string result = "";
        bool in_string = false;

        auto format_char = [](bool& in_string, const char c, const char next)
        {
          auto format_byte = [](const char byte)
          {
            return std::format(" {:02x}", static_cast<unsigned char>(byte));
          };

          if (std::isprint(static_cast<int>(c)))
          {
            if (!in_string)
            {
              if (std::isprint(static_cast<int>(next)))
              {
                in_string = true;
                return std::format(" \"{}", c);
              }
              else
              {
                return format_byte(c);
              }
            }
            else
            {
              return std::string(1, c);
            }
          }
          else
          {
            if (in_string)
            {
              in_string = false;
              return '"' + format_byte(c);
            }
            return format_byte(c);
          }
        };

        std::size_t pos = 0;
        for (; pos < std::min(size - 1, static_cast<std::size_t>(32)); ++pos)
        {
          result += format_char(in_string, static_cast<char>(p[pos]), static_cast<char>(p[pos + 1]));
        }
        result += format_char(in_string, static_cast<char>(p[pos]), 0);
        if (in_string)
          result += '"';
        if (pos < (size - 1))
          result += " <truncated...>";
        return result;
      }


      void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
      {
        // PLOGD << m_name << " DEALLOC: Address: " << p << " Size: " << bytes <<
        //                    " Data: " << format_destroyed_bytes(static_cast<std::byte*>(p), bytes);
        
        // if (m_deallocCheck)
        //   m_deallocCheck(static_cast<std::byte*>(p), bytes);

        m_upstream->deallocate(p, bytes, alignment);
      }

      
      bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
      {
        return this == &other;
      }
    

      private:
        std::size_t m_alloc = 0;
        std::size_t m_dealloc = 0;
        std::function<void(std::byte *, const std::size_t)> m_deallocCheck;
    };
  #endif

  
  class Memory
  {
  private:
    // TODO std::pmr::pool_options, see if there are optimisations for larger chunk sizes,
    //      particularly with blobs which will likely be a factor larger
    #ifdef FC_DEBUG
      Memory() :  m_fixedResource(1024),
                  m_fixedPrint("Value Fixed", &m_fixedResource),
                  m_poolResource(&m_fixedPrint),
                  m_poolPrint("Value Pool", &m_poolResource)
      {
      }
    #else
      Memory() :  m_fixedResource(1024),  // this size is irrelevant: pool determines alloc sizes
                  m_poolResource(&m_fixedResource)
      {
      }
    #endif


  public:
    ~Memory() = default;


    static std::pmr::memory_resource * getPool() noexcept
    {
      return get().pool();
    }

  private:

    static Memory& get()
    {
      static Memory mem;
      return mem;
    }

    std::pmr::memory_resource * pool() noexcept
    {
      #ifdef FC_DEBUG
        return &m_poolPrint; 
      #else
        return &m_poolResource;     
      #endif
    }


  private:
    #ifdef FC_DEBUG
      std::pmr::monotonic_buffer_resource m_fixedResource;
      PrintResource m_fixedPrint;
      std::pmr::unsynchronized_pool_resource m_poolResource;
      PrintResource m_poolPrint;
    #else
      std::pmr::monotonic_buffer_resource m_fixedResource;
      std::pmr::unsynchronized_pool_resource m_poolResource;
    #endif
  };

  
  // This contains the resources used by the cache map (ankerl::unordered_dense::map).
  // This is to ensure the key (std::pmr::string) and value (CachedValue) are placed in
  // pooled memory.
  //
  // This class has some debugging code, which will be removed and tidied. It is used in memtest 
  // to convince myself data is being stored in memory allocated by the pool.
  //
  // The checkF function is called from the PrintResource::do_deallocate(). The checkF() checks through the
  // pool's memory to find particular data (proving the map's value are infact in the pool's memory as expected).
  class MapMemory
  {
  private:
    #ifdef FC_DEBUG
      MapMemory(std::function<void(std::byte *, const std::size_t)> checkF = nullptr) :
                  m_mapFixedResource(m_buffer.data(), m_buffer.size(), std::pmr::null_memory_resource()),
                  m_mapFixedPrint("Map Fixed", &m_mapFixedResource),
                  m_mapPoolResource(&m_mapFixedPrint),
                  m_mapPoolPrint("Map Pool", &m_mapPoolResource, checkF)
      {
      }
    #else
      MapMemory() : m_mapFixedResource(m_buffer.data(), m_buffer.size()),
                    m_mapPoolResource(&m_mapFixedResource)
      {
      }
    #endif


  public:
    ~MapMemory() = default;


    #ifdef FC_DEBUG
      static std::pmr::memory_resource * getPool(std::function<void(std::byte *, const std::size_t)> checkF = nullptr) noexcept
      {
        return get(checkF).pool();
      }
    #else
      static std::pmr::memory_resource * getPool() noexcept
      {
        return get().pool();
      }
    #endif

  private:

    #ifdef FC_DEBUG
      static MapMemory& get(std::function<void(std::byte *, const std::size_t)> checkF)
      {
        static MapMemory mem{checkF};
        return mem;
      }
    #else
      static MapMemory& get()
      {
        static MapMemory mem;
        return mem;
      }
    #endif
    

    std::pmr::memory_resource * pool() noexcept
    {
      #ifdef FC_DEBUG
        return &m_mapPoolPrint; 
      #else
        return &m_mapPoolResource;     
      #endif
    }


  private:
    #ifdef FC_DEBUG
      std::array<std::uint8_t, 32768> m_buffer;
      std::pmr::monotonic_buffer_resource m_mapFixedResource;
      PrintResource m_mapFixedPrint;
      std::pmr::unsynchronized_pool_resource m_mapPoolResource;
      PrintResource m_mapPoolPrint;
    #else
      std::array<std::uint8_t, 32768> m_buffer;
      std::pmr::monotonic_buffer_resource m_mapFixedResource;
      std::pmr::unsynchronized_pool_resource m_mapPoolResource;
    #endif
  };
}
