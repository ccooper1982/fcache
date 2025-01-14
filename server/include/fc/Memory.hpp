#pragma once

#include <memory_resource>
#include <plog/Log.h>
#include <format>

namespace fc
{
  #ifdef FC_DEBUG
    class print_alloc : public std::pmr::memory_resource
    {
    public:
      print_alloc(std::string name, std::pmr::memory_resource* upstream)
          : m_name(std::move(name)),
            m_upstream(upstream)
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
        for (; pos < std::min(size - 1, static_cast<std::size_t>(32)); ++pos) {
          result += format_char(in_string, static_cast<char>(p[pos]),
                                static_cast<char>(p[pos + 1]));
        }
        result += format_char(in_string, static_cast<char>(p[pos]), 0);
        if (in_string)
        {
          result += '"';
        }
        if (pos < (size - 1))
        {
          result += " <truncated...>";
        }
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
    };
  #endif
  

  // TODO std::pmr::pool_options
  class Memory
  {
  private:
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


  public:
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
      print_alloc m_fixedPrint;
      std::pmr::unsynchronized_pool_resource m_poolResource;
      print_alloc m_poolPrint;
    #else
      std::pmr::monotonic_buffer_resource m_fixedResource;
      std::pmr::unsynchronized_pool_resource m_poolResource;
    #endif
  };
}
