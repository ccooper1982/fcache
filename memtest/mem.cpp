
//#define FC_DEBUG  // to enable debug from Memory.hpp
#undef FC_DEBUG

#include <variant>
#include <numeric>
#include <chrono>
#include <list>
#include <fc/Memory.hpp>
#include <fc/LogFormatter.hpp>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <ankerl/unordered_dense.h>


static plog::ColorConsoleAppender<fc::FcFormatter> consoleAppender;


// not PMR aware
struct VectorValue
{
  std::vector<uint8_t> data;
};

struct CV
{
  VectorValue value;
};


// PMR aware
struct VectorValuePmr
{
  using allocator_type = std::pmr::polymorphic_allocator<uint8_t>;
  

  VectorValuePmr() = default;
  VectorValuePmr(const VectorValuePmr&) noexcept = default;
  VectorValuePmr(VectorValuePmr&&) noexcept = default;

  VectorValuePmr& operator= (const VectorValuePmr& other) noexcept = default;
  VectorValuePmr& operator= (VectorValuePmr&& other) noexcept = default;


  explicit VectorValuePmr(const allocator_type& alloc) : data(alloc)
  {

  }

  VectorValuePmr(const std::pmr::vector<uint8_t>& d, const allocator_type& alloc) noexcept :
    data(d, alloc)
  {
    
  }

  VectorValuePmr(std::pmr::vector<uint8_t>&& d, const allocator_type& alloc) noexcept :
    data(std::move(d), alloc)
  {
    
  }

  VectorValuePmr (const VectorValuePmr& other, const allocator_type& alloc) noexcept :
    data(other.data, alloc)
  {    
  }

  VectorValuePmr (VectorValuePmr&& other, const allocator_type& alloc) noexcept :
    data(std::move(other.data))
  {    
  }

  std::pmr::vector<uint8_t> data;
};

  
struct PmrCV
{
  inline static const std::uint8_t FIXED = 0;
  inline static const std::uint8_t VEC   = 1;

  using allocator_type = std::pmr::polymorphic_allocator<>;


  PmrCV() = default;
  PmrCV(const PmrCV&) noexcept = default;
  PmrCV(PmrCV&&) noexcept = default;

  PmrCV& operator= (const PmrCV& other) noexcept = default;
  PmrCV& operator= (PmrCV&& other) noexcept = default;


  explicit PmrCV (const allocator_type& alloc) noexcept
  {    
  }


  PmrCV (const VectorValuePmr& v, const allocator_type& alloc) noexcept :
    value(v, alloc)
  {
  }


  PmrCV (VectorValuePmr&& v, const allocator_type& alloc) noexcept :
    value(std::move(v), alloc)
  {
  }


  PmrCV (const PmrCV& other, const allocator_type& alloc) noexcept : value(other.value)
  {    
  }


  PmrCV (PmrCV&& other, const allocator_type& alloc) noexcept :
    value(std::move(other.value))
  {    
  }

  VectorValuePmr value;
};
  


/*
static void checkMemory (std::byte * buffer, const std::size_t size)
{
  PLOGE << "Checking Buffer: " << buffer << ", size: " << size;

  for (std::size_t i = 0 ; i < size ; )
  {
    if (static_cast<int>(buffer[i]) == 10 && i+31 < size)
    {
      if (static_cast<int>(buffer[i+8]) == 11 &&
          static_cast<int>(buffer[i+16]) == 12 &&
          static_cast<int>(buffer[i+24]) == 13)
      {
        PLOGE << "Found values in buffer: " << buffer;
        i = size; // force exit
      }      
    }
    else if (static_cast<char>(buffer[i]) == 'k' && i+1 < size)
    {
      if (static_cast<char>(buffer[i+1]) == '2')
      {
        PLOGE << "Found k2";
        i = size;
      }
    }
    else if (static_cast<char>(buffer[i]) == 'l' && static_cast<char>(buffer[i+1]) == 'o' && i+23 < size)
    {
      std::string_view sv{reinterpret_cast<char *>(buffer+i), 23};
      PLOGE << sv;
    }
    
    ++i;
  }
}
*/



using clok = std::chrono::steady_clock;
//using PmrMap = ankerl::unordered_dense::pmr::map<std::pmr::string, PmrCV>;
using PmrMap = ankerl::unordered_dense::pmr::map<std::string, PmrCV>;
using Map = ankerl::unordered_dense::map<std::string, CV>;


struct Timer
{
  Timer(const std::string_view name = "") : s(clok::now()), name(name)
  {
  }

  ~Timer()
  {
    PLOGI << name << ": " << (clok::now() - s).count();
  }

  clok::time_point s;
  std::string name;
};



void perfPmr(const uint64_t nKeys, const uint64_t nValuesPerKey)
{
  std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool()};
  PmrMap map(alloc);


  // warm up the pool: the pool will allocate memory. We can't do this with non-PMR version.
  // we then clear the map, making the pool memory available for subsequent keys, with
  // memory already allocated.
  for (uint64_t i = 0 ; i < nKeys ; ++i)
  {
    //auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, VectorValuePmr{});
    auto [it, emplaced] = map.try_emplace(std::to_string(i), VectorValuePmr{});
    auto& vec = it->second.value.data;
    vec.resize(nValuesPerKey*sizeof(uint64_t));
    // don't add anything to the map, we've resized which forces the pool to allocate memory
  }

  map.clear();

  // bunch of keys we'll later retrieve. build here rather than in the
  // loop below to avoid skewing timing.
  std::vector<std::string> keys;
  keys.reserve(nKeys);
  for(uint64_t i =0 ; i < nKeys ; ++i)
    //keys.emplace_back(std::pmr::string{std::to_string(i), alloc});
    keys.emplace_back(std::to_string(i));
  

  int total = 0;
  
  {
    Timer t{"PMR Set 1"};
  
    for (uint64_t i = 0 ; i < nKeys ; ++i)
    {
      //auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, VectorValuePmr{});
      auto [it, emplaced] = map.try_emplace(std::to_string(i), VectorValuePmr{});
      auto& vec = it->second.value.data;
      vec.resize(nValuesPerKey*sizeof(uint64_t));

      for (uint64_t i = 0, j = 0; i < nValuesPerKey ; ++i)
      {
        std::memcpy(vec.data()+j, &i, sizeof(uint64_t));
        j += sizeof(uint64_t);
      }
    }
  }

  {
    Timer t{"PMR Get"};

    for (const auto& k : keys)
    {
      if (const auto it = map.find(k); it != map.cend())
      {
        const auto& values = it->second.value.data;
        total = std::accumulate(values.cbegin(), values.cend(), 0);
      }
    }
  }

  // delete keys then add more
  for (uint64_t i = 0 ; i < nKeys/10 ; ++i)
    //map.erase(std::pmr::string(std::to_string(i), alloc));
    map.erase(std::to_string(i));
  
  {
    Timer t{"PMR Set 2"};
  
    for (uint64_t i = 0 ; i < nKeys/10 ; ++i)
    {
      //auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, VectorValuePmr{});
      auto [it, emplaced] = map.try_emplace(std::to_string(i), VectorValuePmr{});

      auto& vec = it->second.value.data;
      vec.resize(nValuesPerKey*sizeof(uint64_t));

      for (uint64_t i = 0, j = 0; i < nValuesPerKey ; ++i)
      {
        std::memcpy(vec.data()+j, &i, sizeof(uint64_t));
        j += sizeof(uint64_t);
      }
    }
  }

  PLOGI << total;
}

/*  Commented because ListMemory resource removed:

// https://quick-bench.com/q/GEtDNQybceEUt1cGsxRBDzwF22c
//  Suggests that using PMR for list isn't beneficial, perhaps:
//  - The pool creates blocks of various sizes
//  - But with a list of scalar type, the size will be the same for fixed types (list<int>, list<float>, etc)
//  - A custom pool allocator may be worthwhile: similar to std library pool but all blocks can have the same
//    chunk size on a per list basis: the chunk size is 
//      - node * prev
//      - node * next
//      - T val;
//   plus any padding for alignment.

void perfNormal(const uint64_t nKeys, const uint64_t nValuesPerKey)
{
  std::vector<std::string> keys(nKeys);
  for(uint64_t i =0 ; i < nKeys ; ++i)
    keys[i] = std::to_string(i);

  Map map;

  int total = 0;
  {
    Timer t{"Set 1"};

    for (uint64_t i = 0 ; i < nKeys ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::to_string(i), VectorValue{});

      auto& vec = it->second.value.data;
      vec.resize(nValuesPerKey*sizeof(uint64_t));

      for (uint64_t i = 0, j = 0; i < nValuesPerKey ; ++i)
      {
        std::memcpy(vec.data()+j, &i, sizeof(uint64_t));
        j += sizeof(uint64_t);
      }
    }
  }


  {  
    Timer t{"Get"};

    for (const auto& k : keys)
    {
      const auto& values = map.at(k).value.data;
      total = std::accumulate(values.cbegin(), values.cend(), 0);
    }
  }


  // delete keys then add more
  for (uint64_t i = 0 ; i < nKeys/10 ; ++i)
    map.erase(std::to_string(i));


  {
    Timer t{"Set 2"};
  
    for (uint64_t i = 0 ; i < nKeys ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::to_string(i), VectorValue {});

      auto& vec = it->second.value.data;
      vec.resize(nValuesPerKey*sizeof(uint64_t));

      for (uint64_t i = 0, j = 0; i < nValuesPerKey ; ++i)
      {
        std::memcpy(vec.data()+j, &i, sizeof(uint64_t));
        j += sizeof(uint64_t);
      }
    }
  }
  
  PLOGI << total;    
}


void perfList(uint64_t nNodes)
{
  std::list<uint64_t> list1, list2;

  {
    Timer {"List Create"};

    for (uint64_t i  = 0 ; i < nNodes ; ++i)
    {
      list1.emplace_back(i);
      if (i % 2 == 0)
        list2.emplace_back(i);
    }    
  }
  

  {
    Timer {"List Intersect"};

    std::vector<uint64_t> result;
    result.reserve(nNodes);
    std::set_intersection(std::cbegin(list1), std::cend(list1),
                          std::cbegin(list2), std::cend(list2), std::back_inserter(result));
  }

  // remove half
  {
    for (uint64_t i = 0 ; i  < nNodes/2 ; ++i)
      list1.erase(list1.cbegin());
  }

  // add back
  {
    Timer {"List Add"};
    
    for (uint64_t i  = 0 ; i < nNodes/2 ; ++i)
      list1.emplace_back(i);
  }
}


struct PmrList
{
  PmrList () : list(memory.alloc())
  {

  }
  
  fc::ListMemory memory;
  std::pmr::list<uint64_t> list;
};


void perfListPmr(uint64_t nNodes)
{
  PmrList list1, list2;

  
  {
    Timer {"PMR List Create"};
    
    for (uint64_t i  = 0 ; i < nNodes ; ++i)
    {
      list1.list.emplace_back(i);
      if (i % 2 == 0)
        list2.list.emplace_back(i);
    } 
  }

  {
    Timer {"PMR List Intersect"};
    
    std::vector<uint64_t> result;
    result.reserve(nNodes);
    std::set_intersection(std::cbegin(list1.list), std::cend(list1.list),
                          std::cbegin(list2.list), std::cend(list2.list), std::back_inserter(result));  
  }
  
  // remove half
  {
    for (uint64_t i = 0 ; i  < nNodes/2 ; ++i)
      list1.list.erase(list1.list.cbegin());
  }

  // add back
  {
    Timer {"PMR List Add"};

    for (uint64_t i  = 0 ; i < nNodes/2 ; ++i)
      list1.list.emplace_back(i);
  }
}
*/


int main (int argc, char ** argv)
{
  plog::init(plog::verbose, &consoleAppender);


  #ifdef FC_DEBUG
    // std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool(checkMemory)};
    // PmrMap map(alloc);

    // IntVector v{{10,11,12,13}, alloc};
    // StringVector v2{{"long_string_beats_sso_1"}, alloc};

    // map.emplace("k2", v);
    // map.emplace("k3", v2);
  #endif
  
  // perfNormal(100000, 10);
  // perfPmr(100000, 10);
  
  //perfList(10000);
  //perfListPmr(10);

  return 0;
}
