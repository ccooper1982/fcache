#pragma once

#include <memory_resource>
#include <plog/Log.h>
#include <format>

namespace fc
{
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


      std::size_t nAlloc() const noexcept
      {
        return m_alloc;
      }

      std::size_t nDealloc() const noexcept
      {
        return m_dealloc;
      }

    private:
      std::string m_name;
      std::pmr::memory_resource * m_upstream;

      void * do_allocate(std::size_t bytes, std::size_t alignment) override
      {
        m_alloc += bytes;

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
        m_dealloc += bytes;

        PLOGD << m_name << " DEALLOC: Address: " << p << " Size: " << bytes <<
                           " Data: " << format_destroyed_bytes(static_cast<std::byte*>(p), bytes);
            
        m_upstream->deallocate(p, bytes, alignment);
      }

      
      bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
      {
        return this == &other;
      }
    



      private:
        std::size_t m_alloc = 0;
        std::size_t m_dealloc = 0;
    };
  #endif


  /// Uses a pool resource backed by monotonic resource:
  ///   - when the pool requires memory it requests a buffer from the monotonic resource
  ///   - a mononotic resource only releases memory when the resource is destroyed
  ///   - but that's ok because the pool tracks unused memory (free list) within the buffer allocated by the monotonic
  ///   - when a key is erased, the free list is aware and can reuse it
  ///
  /// This has been influenced by Jason Turner's video:
  ///   https://www.youtube.com/watch?v=Zt0q3OEeuB0&list=PLs3KjaCtOwSYX3X0L36NgwK0pxZZavDSF&index=5&t=265
  /// Specifically from 6mins45.
  ///
  /// - The pool resource allocates an internal structure to track pools
  /// - The pool resource contains multiple pools, each intended for different size requirements
  /// - Each of those pools contains multiple blocks (which are logically split into chunks)
  class Memory
  {
  private:
    // TODO std::pmr::pool_options, see if there are optimisations for larger chunk sizes,
    //      particularly with blobs which will likely be a factor larger
    #ifdef FC_DEBUG
      Memory() :  m_fixedResource(1024),  // this size is irrelevant: pool determines alloc sizes
                  m_fixedPrint("Fixed", &m_fixedResource),
                  m_poolResource(&m_fixedPrint),
                  m_poolPrint("Pool", &m_poolResource)
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


    #ifdef FC_DEBUG
      static std::size_t nAllocated() noexcept
      {
        return get().m_fixedPrint.nAlloc() + get().m_poolPrint.nAlloc();
      }

      static std::size_t nDeallocated() noexcept
      {
        return get().m_fixedPrint.nDealloc() + get().m_poolPrint.nDealloc();
      }

      static std::size_t net() noexcept
      {
        // this is slightly pointless because net() is always > 0: nAllocated() includes memory
        // allocated by the pool to manage the blocks, and isn't entirely released until the Memory   
        // singleton is destroyed. The value of net() after all resource users are destroyed is most likely the 
        // memory used by the pool to manage the pools.
        // valgrind shows all memory is released.
        return nAllocated() - nDeallocated();
      }
    #endif


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
}
