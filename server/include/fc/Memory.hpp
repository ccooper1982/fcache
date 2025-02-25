#pragma once

#include <memory_resource>
#include <functional>
#include <plog/Log.h>
#include <format>
#include <fc/Common.hpp>

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
      PrintResource(std::string name, std::pmr::memory_resource* upstream)
          : m_name(std::move(name)),
            m_upstream(upstream)
      {
        assert(upstream);
      }


    private:
      

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
        PLOGD << m_name << " DEALLOC: Address: " << p << " Size: " << bytes <<
                           " Data: " << format_destroyed_bytes(static_cast<std::byte*>(p), bytes);
        
        m_upstream->deallocate(p, bytes, alignment);
      }

      
      bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
      {
        return this == &other;
      }
    

      private:
        std::string m_name;
        std::pmr::memory_resource * m_upstream;
        std::size_t m_alloc = 0;
        std::size_t m_dealloc = 0;
    };
  #endif

  

  // This contains the resources used by the cache map (ankerl::unordered_dense::map).
  // The key (std::pmr::string) and value (CachedValue) are placed in pooled memory.
  // If the CachedValue contains a VectorValue, it uses the VectorMemory below.
  class MapMemory
  {
  private:
    #ifdef FC_DEBUG
      MapMemory() : m_mapFixedResource(1024),
                    m_mapFixedPrint("Map Mono", &m_mapFixedResource),
                    m_mapPoolResource(&m_mapFixedPrint),
                    m_mapPoolPrint("Map Pool", &m_mapPoolResource)
      {
      }
    #else
      MapMemory() : m_mapFixedResource(1024),
                    m_mapPoolResource(&m_mapFixedResource)
      {
      }
    #endif


  public:
    ~MapMemory() = default;

    static std::pmr::memory_resource * getPool() noexcept
    {
      return get().pool();
    }

  private:

    static MapMemory& get()
    {
      static MapMemory mem;
      return mem;
    }
    

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
      std::pmr::monotonic_buffer_resource m_mapFixedResource;
      PrintResource m_mapFixedPrint;
      std::pmr::unsynchronized_pool_resource m_mapPoolResource;
      PrintResource m_mapPoolPrint;
    #else
      std::pmr::monotonic_buffer_resource m_mapFixedResource;
      std::pmr::unsynchronized_pool_resource m_mapPoolResource;
    #endif
  };


  // This pool is used by VectorValue to store vectors of scalars types,
  // and also strings (treated as a vector chars).
  class VectorMemory
  {
  private:
    // TODO std::pmr::pool_options, see if there are optimisations for larger chunk sizes,
    //      particularly with blobs which will likely be a factor larger
    #ifdef FC_DEBUG
      VectorMemory() :  m_fixedResource(1024),
                        m_fixedPrint("Value Mono", &m_fixedResource),
                        m_poolResource(&m_fixedPrint),
                        m_poolPrint("Value Pool", &m_poolResource),
                        m_alloc(&m_poolResource)
      {
      }
    #else
      VectorMemory() :  m_fixedResource(1024),  // this size is irrelevant: pool determines alloc sizes
                        m_poolResource(&m_fixedResource),
                        m_alloc(&m_poolResource)
      {
      }
    #endif


  public:
    ~VectorMemory() = default;


    static std::pmr::memory_resource * getPool() noexcept
    {
      return get().pool();
    }

    static std::pmr::polymorphic_allocator<>& alloc() noexcept
    {
      return get().m_alloc;
    }

  private:

    static VectorMemory& get()
    {
      static VectorMemory mem;
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

    std::pmr::polymorphic_allocator<> m_alloc;
  };

  
}
