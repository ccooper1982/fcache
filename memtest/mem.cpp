
#define FC_DEBUG  // to enable debug from Memory.hpp

#include <variant>
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


template<typename T>
using Vector = std::pmr::vector<T>;

using IntVector = Vector<std::int64_t>;
using UIntVector = Vector<std::uint64_t>;
using FloatVector = Vector<float>;
using BoolVector = Vector<bool>;
using CharVector = Vector<char>; 
using StringVector = Vector<std::pmr::string>;  // TODO, perhaps Vector<CharVector>
using BlobVector = Vector<uint8_t>;

// emulating CachedValue from fcache
struct CV
{
  using allocator_type = std::pmr::polymorphic_allocator<>;

  CV() = default;
  CV(const CV&) = default;
  CV(CV&&) = default;

  explicit CV (const std::pmr::polymorphic_allocator<>& alloc)
  {    
  }

  CV (const IntVector& v, const std::pmr::polymorphic_allocator<>& alloc) : value(v)
  {
  }

  CV (const CV& other, const std::pmr::polymorphic_allocator<>& alloc) : value(other.value)
  {    
  }

  CV (CV&& other, const std::pmr::polymorphic_allocator<>& alloc) : value(std::move(other.value))
  {    
  }

  CV& operator= (const CV& other) = default;
  CV& operator= (CV&& other) = default;

  std::variant<IntVector> value;
};


/*
The first alloc is 528 bytes, which is likely the pool's internal containers,
used to track the pools.
*/


void general()
{
  // TODO confirm why segmented_map does not compile, and if that's by design
  using Map = ankerl::unordered_dense::pmr::map<std::pmr::string, Value>;

  PLOGE << "Creating Map";
  {
    std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool()};
    Map map{alloc};

    PLOGE << "Emplacing 1";
    map.emplace(std::pmr::string("key1",fc::MapMemory::getPool()), "THIS is a long string 1");

    if (auto it = map.find("key1") ; it != map.end())
    {
      PLOGE << "Replacing";
      it->second.s = "THIS IS STRING NEW VALUE";
    }
    
    // PLOGE << "Emplacing";
    // map.emplace("key2", "THIS is a long string 2");
    // PLOGE << "Emplacing";
    // map.emplace("key3", "THIS is a long string 3");

    PLOGE << "Destroying Map";
  }

  // PLOGE << "Creating Map Loop";
  // {
  //   std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool()};
  //   Map map{alloc};

  //   std::vector<std::string> keys{"k1", "k2", "k3", "k4", "k5", "k6", "k7", "k8"};
    
  //   PLOGE << "Begin loop";
  //   for (const auto& k : keys)
  //   {
  //     PLOGE << "Emplacing";
  //     map.emplace(k, "THIS IS A LONG STRING");
  //   }
      
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


static void checkMemory (std::byte * buffer, const std::size_t size)
{
  PLOGE << "Checking Buffer: " << buffer << ", size: " << size;

  for (std::size_t i = 0 ; i < size ; )
  {
    if (static_cast<int>(buffer[i]) == 1 && i+31 < size)
    {
      PLOGE << "size: " << size;
      if (static_cast<int>(buffer[i+8]) == 2 &&
          static_cast<int>(buffer[i+16]) == 3 &&
          static_cast<int>(buffer[i+24]) == 4)
      {
        PLOGE << "Found values in buffer: " << buffer;
        i = size; // force exit
      }      
    }
    else if (static_cast<char>(buffer[i]) == 'k' && i+1 < size)
    {
      if (static_cast<char>(buffer[i+1]) == '1')
      {
        PLOGE << "Found k1";
        i = size;
      }
    }
    
    ++i;
  }
}


int main (int argc, char ** argv)
{
  plog::init(plog::verbose, &consoleAppender);


  //general();


  using Map = ankerl::unordered_dense::pmr::map<std::pmr::string, CV>;

  std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool(checkMemory)};
  Map map(alloc);

  IntVector v{{1,2,3,4}, alloc};

  map.emplace("k1", v);

  return 0;
}
