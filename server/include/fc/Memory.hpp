#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>
#include <plog/Log.h>


namespace fc
{
  using BlockId = std::uint32_t;
  using BlockPos = std::uint16_t;
  using BlockBuffSize = std::uint16_t;


  struct Block
  {
    Block(const std::size_t capacity) : capacity(capacity), used(0)
    {
      buffer = std::make_unique<char[]>(capacity);
    }

    bool hasCapacity(const std::size_t n) const noexcept
    {
      return used + n <= capacity;
    }

    template<typename T>
    BlockPos write (const T& v)
    {
      const auto start = used;
      overwrite(used, v);
      used += sizeof(T);
      return start;
    }


    template<typename T>
    void overwrite (const BlockPos pos, const T& v)
    {
      std::memcpy(std::next(buffer.get(), used), &v, sizeof(T));
    }


    const char * getRead(const BlockPos pos) const 
    {
      return std::next(buffer.get(), pos);
    }


    void clear(const BlockPos pos, const BlockBuffSize size)
    {
      // if this data is last in the buffer
      //  [--------------####]
      //         ^------^
      //        pos     | free
      //  [-------############]
      // we can reduce used ,
      
      // otherwise do nothing, the key that refers to this will be deleted anyway.
      // could improve this by tracking unused regions, when we find unused 
      // regions adjacent to the last used position, we can release that region:
      // #:released, X:never used, -:in-use
      // 
      //  [--------####----XXXXXXXX]
      //               ^  ^ 
      //               clear
      // if these last 4 bytes are cleared:
      //  [--------########XXXXXXXX]
      // from # to the last X are available
      if (pos+size == used)
        used -= size;
    }

    private:
      std::unique_ptr<char[]> buffer;
      BlockBuffSize capacity;    
      BlockBuffSize used;
  };


  struct BlockView
  {
    BlockId id;
    BlockPos pos;
    std::size_t size;
  };


  class FixedMemory
  {
    inline const static std::uint32_t InitialBlocks = 32;
    inline const static BlockBuffSize BlockCapacity = 1024;


  public:

    ~FixedMemory ()
    {
      for (auto block : m_blocks)
        delete block;
    }

    
    bool init ()
    {
      try
      {
        PLOGD << "FixedMemory: initialising with " << InitialBlocks << " blocks @ " << BlockCapacity << " bytes";

        for (std::size_t i = 0 ; i < InitialBlocks ; ++i)
          m_blocks.emplace_back(new Block{BlockCapacity});
      }
      catch(const std::exception& e)
      {
        PLOGE << "FixedMemory failed to initialise. InitialBlocks: "
              << InitialBlocks << ", BlockCapacity: " << BlockCapacity;

        return false;
      }
      
      return true;
    }


    // TODO do write() and overwrite() need to return size? The caller should always know
    template<typename T>
    BlockView write(const T v) noexcept requires (std::is_integral_v<T> || std::is_same_v<T, float>) 
    {
      constexpr const std::size_t Size = sizeof(T);
      
      const auto [block, id] = nextBlock(Size);
      
      BlockView bv;
      bv.id = id;
      bv.pos = block->write(v);
      bv.size = Size;
      return bv;
    }


    template<typename T>
    void overwrite(BlockView& bv, const T v)  requires (std::is_integral_v<T> || std::is_same_v<T, float>)
    {
      constexpr const std::size_t NewSize = sizeof(T);

      if (NewSize > bv.size)
      {
        // grab next with sufficient space or create new
        const auto [block, id] = nextBlock(NewSize);
        bv.pos = block->write(v);
        bv.id = id;
        bv.size = NewSize;
      }
      else
      {
        m_blocks[bv.id]->overwrite(bv.pos, v);
      }
    }


    template<typename T>
    const T get(const BlockView& bv) const requires (std::is_integral_v<T> || std::is_same_v<T, float>)
    {
      T v{};
      std::memcpy(&v, m_blocks[bv.id]->getRead(bv.pos), sizeof(T)); 
      return v;
    }


    void clear(const BlockView& bv)
    {
      m_blocks[bv.id]->clear(bv.pos, bv.size);
    }


  private:
    Block * addBlock()
    {
      PLOGD << "Creating new block: " << BlockCapacity << " bytes";

      Block * const b = new Block{BlockCapacity};
      m_blocks.emplace_back(b);
      return b;
    }


    std::tuple<Block *, BlockId> nextBlock(const std::size_t size)
    {
      Block * block = nullptr;
      BlockId id = 0;

      for ( ; !block && id < m_blocks.size() ; ++id)
      {
        if (m_blocks[id]->hasCapacity(size))
          block = m_blocks[id];
      }
      
      if (block)
        return {block,id};
      else
        return {addBlock(), m_blocks.size()-1};
    }


  private:
    std::vector<Block *> m_blocks;
  };


  // for future use, just to allow CachedValue to compile
  class VariedMemory
  {
    
  };
  
}