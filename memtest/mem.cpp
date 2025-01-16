
#define FC_DEBUG  // to enable debug from Memory.hpp

#include <fc/Memory.hpp>
#include <fc/LogFormatter.hpp>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <ankerl/unordered_dense.h>


static plog::ColorConsoleAppender<fc::FcFormatter> consoleAppender;


struct Value
{
  using allocator_type = std::pmr::polymorphic_allocator<>;

  Value() = default;
  Value(const Value&) = default;
  Value(Value&&) = default;

  explicit Value (const std::pmr::polymorphic_allocator<>& alloc) : s(alloc)
  {    
  }

  explicit Value (const char * str, const std::pmr::polymorphic_allocator<>& alloc) : s(str, alloc)
  {

  }

  Value (const Value& other, const std::pmr::polymorphic_allocator<>& alloc) : s(other.s, alloc)
  {    
  }

  Value (Value&& other, const std::pmr::polymorphic_allocator<>& alloc) : s(std::move(other.s), alloc)
  {    
  }


  Value& operator= (const Value& other)
  {
    if (this != &other)
      s = other.s;
    return *this;
  }

  Value& operator= (Value&& other)
  {
    if (this != &other)
      s = std::move(other.s);
    return *this;
  }

  std::pmr::string s;
};

/*
The first alloc is 528 bytes, which is likely the pool's internal containers,
used to track the pools.
*/

int main (int argc, char ** argv)
{
  plog::init(plog::verbose, &consoleAppender);
  
  PLOGE << "Creating Map";
  {
    std::pmr::polymorphic_allocator alloc{fc::MapMemory::getMapPool()};
    // NOTE need to use map, not segment_map:
    //      segment_map uses a segmented_vector
    ankerl::unordered_dense::pmr::map<std::string, Value> map{alloc};

    PLOGE << "Emplacing";
    map.emplace("key1", "THIS is a long string 1");
    PLOGE << "Emplacing";
    map.emplace("key2", "THIS is a long string 2");
    PLOGE << "Emplacing";
    map.emplace("key3", "THIS is a long string 3");

    PLOGE << "Destroying Map";
  }

  // PLOGE << "Creating Map Loop";
  // {
  //   std::pmr::polymorphic_allocator alloc{fc::MapMemory::getMapPool()};
  //   ankerl::unordered_dense::pmr::map<std::string, Value> map{alloc};

  //   PLOGE << "Begin loop";
    
  //   PLOGE << "End loop";
  // }
  


  // PLOGE << "String";
  // {
  //   PLOGE << "Creating";
  //   std::pmr::vector<std::pmr::string> v(fc::Memory::getPool());

  //   PLOGE << "Emplacing";
  //   v.emplace_back("Hello World");

  //   PLOGE << "Destroying";
  // }

  // PLOGD << "------------";

  // PLOGE << "vector<char>";
  // {
  //   PLOGE << "Creating";
  //   std::pmr::vector<char> v(fc::Memory::getPool());

  //   PLOGE << "Emplacing";
  //   v.emplace_back('H');
  //   v.emplace_back('E');
  //   v.emplace_back('L');
  //   v.emplace_back('L');
  //   v.emplace_back('O');

  //   PLOGE << "Destroying";
  // }

  // PLOGD << "------------";

  // PLOGE << "Blob";
  // {
  //   PLOGE << "Creating";
  //   std::pmr::vector<uint8_t> v(fc::Memory::getPool());
    
  //   PLOGE << "Inserting";
  //   v.insert(v.cbegin(), 11'030, 0);

  //   PLOGE << "Destroying";
  // }

  // PLOGD << "Allocated: " << fc::Memory::nAllocated() << ", Deallocated: " << fc::Memory::nDeallocated();
}
