#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>
#include <plog/Log.h>


namespace fc
{
  struct Block
  {
    std::unique_ptr<char[]> buffer;
    std::size_t capacity;    
    std::size_t used;

    Block(const std::size_t capacity) : capacity(capacity), used(0)
    {
      buffer = std::make_unique<char[]>(capacity);
    }

    bool hasCapacity(const std::size_t n) const noexcept
    {
      return used + n <= capacity;
    }

    template<typename T>
    char * write (const T& v)
    {
      const auto pos = std::next(buffer.get(), used);
      overwrite(pos, v);
      used += sizeof(T);
      return pos;
    }

    template<typename T>
    void overwrite (char * pos, const T& v)
    {
      std::memcpy(pos, &v, sizeof(T));
    }
  };


  class FixedMemory
  {
    inline const static std::size_t InitialBlocks = 10;
    inline const static std::size_t BlockCapacity = 1024;


  public:

    ~FixedMemory ()
    {
      for (auto& block : m_blocks)
        delete block;
    }

    
    bool init ()
    {
      try
      {
        for (std::size_t i = 0 ; i < InitialBlocks ; ++i)
          m_blocks.push_back(new Block{BlockCapacity});
      }
      catch(const std::exception& e)
      {
        PLOGE << "FixedMemory failed to initialise. InitialBlocks: "
              << InitialBlocks << ", BlockCapacity: " << BlockCapacity;

        return false;
      }
      
      return true;
    }


    template<typename T>
    char * write(const T v) noexcept requires (std::is_integral_v<T> || std::is_same_v<T, float>) 
    {
      constexpr const std::size_t Size = sizeof(T);
      
      Block * block = nextBlock(Size);
      return block->write(v);
    }


    template<typename T>
    char * overwrite(char * pos, const T v, const std::size_t existingSize) requires (std::is_integral_v<T> || std::is_same_v<T, float>)
    {
      if (sizeof(T) > existingSize)
      {
        // take easy route: grab next available 
        // may be something better possible
        Block * block = nextBlock(sizeof(T));
        return block->write(v);
      }
      else
      {
        std::memcpy(pos, &v, sizeof(T));
        return pos;
      }
    }


    template<typename T>
    const T get(char * buff) requires (std::is_integral_v<T> || std::is_same_v<T, float>)
    {
      T v{};

      if constexpr (std::is_same_v<double, T>)
      {
        char x[sizeof(double)+1] = {(char)0};
        std::memcpy(x, buff, 4);
        v = std::atof(x);
        PLOGE << v;
        PLOGE << (float)v;
      }
      else
      {
        std::memcpy(&v, buff, sizeof(T));
      }
      return v;
    }


  private:
    Block * addBlock()
    {
      Block * const b = new Block{BlockCapacity};
      m_blocks.emplace_back(b);
      return b;
    }

    Block * nextBlock(const std::size_t size)
    {
      Block * block = nullptr;
      for (std::size_t i = 0 ; !block && i < m_blocks.size() ; ++i)
      {
        if (m_blocks[i]->hasCapacity(size))
          block = m_blocks[i];
      }
      
      return block ? block : addBlock();
    }

  private:
    std::vector<Block *> m_blocks;
  };
}