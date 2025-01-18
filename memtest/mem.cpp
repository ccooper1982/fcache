
//#define FC_DEBUG  // to enable debug from Memory.hpp
#undef FC_DEBUG

#include <variant>
#include <numeric>
#include <chrono>
#include <fc/Memory.hpp>
#include <fc/LogFormatter.hpp>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <ankerl/unordered_dense.h>


static plog::ColorConsoleAppender<fc::FcFormatter> consoleAppender;


template<typename T>
using Vector = std::pmr::vector<T>;

using IntVector = Vector<std::int64_t>;
using UIntVector = Vector<std::uint64_t>;
using FloatVector = Vector<float>;
using BoolVector = Vector<bool>;
using CharVector = Vector<char>; 
using StringVector = Vector<std::pmr::string>;  // TODO, perhaps Vector<CharVector>
using BlobVector = Vector<uint8_t>;



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
using PmrMap = ankerl::unordered_dense::pmr::map<std::pmr::string, PmrCV>;
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


template<typename M>
void dump (const M& m)
{
  for (const auto& pair : m)
  {
    if constexpr (std::is_same_v<M, PmrMap>)
      PLOGE << pair.first << "=" << std::get<IntVector>(pair.second.value);
    else
      PLOGE << pair.first << "=" << std::get<std::vector<int64_t>>(pair.second.value);
  }
    
}


void perfPmr(const uint64_t nKeys, const uint64_t nValuesPerKey)
{
  std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool()};
  PmrMap map(alloc);


  // warm up the pool: the pool will allocate memory. We can't do this with non-PMR version.
  // we then clear the map, making the pool memory available for subsequent keys, with
  // memory already allocated.
  for (uint64_t i = 0 ; i < nKeys ; ++i)
  {
    auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, VectorValuePmr{});
    auto& vec = it->second.value.data;
    vec.resize(nValuesPerKey*sizeof(uint64_t));
    // don't add anything to the map, we've resized which forces the pool to allocate memory
  }

  map.clear();

  // bunch of keys we'll later retrieve. build here rather than in the
  // loop below to avoid skewing timing.
  std::vector<std::pmr::string> keys;
  keys.reserve(nKeys);
  for(uint64_t i =0 ; i < nKeys ; ++i)
    keys.emplace_back(std::pmr::string{std::to_string(i), alloc});
  

  int total = 0;
  
  {
    Timer t{"PMR Set 1"};
  
    for (uint64_t i = 0 ; i < nKeys ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, VectorValuePmr{});

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
    map.erase(std::pmr::string(std::to_string(i), alloc));
  
  {
    Timer t{"PMR Set 2"};
  
    for (uint64_t i = 0 ; i < nKeys/10 ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, VectorValuePmr{});

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
  
  perfNormal(100000, 10);
  perfPmr(100000, 10);

  return 0;
}
