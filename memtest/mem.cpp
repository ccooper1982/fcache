
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


// emulating CachedValue from fcache but PMR aware
struct PmrCV
{
  using allocator_type = std::pmr::polymorphic_allocator<>;

  PmrCV() = default;
  PmrCV(const PmrCV&) = default;
  PmrCV(PmrCV&&) noexcept = default;
  
  PmrCV& operator= (const PmrCV& other) noexcept = default;
  PmrCV& operator= (PmrCV&& other) noexcept = default;


  explicit PmrCV (const allocator_type& alloc) noexcept
  {    
  }

  PmrCV (const IntVector& v, const allocator_type& alloc) noexcept : value(v)
  {
    //value = IntVector(v, alloc);
    //PLOGI << "Here";
  }

  PmrCV (const StringVector& v, const allocator_type& alloc) noexcept : value(v)
  {
  }

  PmrCV (const PmrCV& other, const allocator_type& alloc) noexcept : value(other.value)
  {    
  }

  PmrCV (PmrCV&& other, const allocator_type& alloc) noexcept : value(std::move(other.value))
  {    
  }

  std::variant<IntVector, StringVector> value;
};


// not PMR aware
struct CV
{
  std::variant<std::vector<int64_t>, std::vector<std::string>> value;
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
    duration = clok::now() - s;
    PLOGI << name << ": " << duration.count();
  }

  clok::time_point s, e;
  clok::duration duration;
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


void perfPmr(const int nKeys, const int nValuesPerKey)
{
  std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool()};
  PmrMap map(alloc);


  // warm up the pool: the pool will allocate memory. We can't do this with non-PMR version.
  // we then clear the map, making the pool memory available for subsequent keys, with
  // memory already allocated.
  for (auto i = 0 ; i < nKeys ; ++i)
  {
    auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, IntVector{fc::Memory::getPool()});
    auto& vec = std::get<IntVector>(it->second.value);
    vec.resize(nValuesPerKey);
    // don't add anything to the map, we've resized which forces the pool to allocate memory
  }

  map.clear();

  // bunch of keys we'll later retrieve. build here rather than in the
  // loop below to avoid skewing timing.
  std::vector<std::pmr::string> keys;
  keys.reserve(nKeys);
  for(auto i =0 ; i < nKeys ; ++i)
    keys.emplace_back(std::pmr::string{std::to_string(i), alloc});
  

  int total = 0;
  
  {
    Timer t{"PMR Set 1"};
  
    for (auto i = 0 ; i < nKeys ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, IntVector{fc::Memory::getPool()});

      auto& vec = std::get<IntVector>(it->second.value);
      vec.resize(nValuesPerKey);

      int c = 0;
      std::generate_n(vec.begin(), nValuesPerKey, [&c]{return c++;});
    }
  }

  {
    Timer t{"PMR Get"};

    for (const auto& k : keys)
    {
      if (const auto it = map.find(k); it != map.cend())
      {
        const auto& values = std::get<IntVector>(it->second.value);
        total = std::accumulate(values.cbegin(), values.cend(), 0);
      }
    }
  }

  // delete keys then add more
  for (int i = 0 ; i < nKeys/10 ; ++i)
    map.erase(std::pmr::string(std::to_string(i), alloc));
  
  {
    Timer t{"PMR Set 2"};
  
    for (auto i = 0 ; i < nKeys/10 ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::pmr::string{std::to_string(i), alloc}, IntVector{fc::Memory::getPool()});

      auto& vec = std::get<IntVector>(it->second.value);
      vec.resize(nValuesPerKey);

      int c = 0;
      std::generate_n(vec.begin(), nValuesPerKey, [&c]{return c++;});
    }
  }

  PLOGI << total;
}


void perfNormal(const int nKeys, const int nValuesPerKey)
{
  std::vector<std::string> keys(nKeys);
  for(auto i =0 ; i < nKeys ; ++i)
    keys[i] = std::to_string(i);

  Map map;

  int total = 0;
  {
    Timer t{"Set 1"};

    for (auto i = 0 ; i < nKeys ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::to_string(i), std::vector<int64_t> {});

      auto& vec = std::get<std::vector<int64_t>>(it->second.value);
      vec.resize(nValuesPerKey);

      int c = 0;
      std::generate_n(vec.begin(), nValuesPerKey, [&c]{return c++;});
    }
  }

  {  
    Timer t{"Get"};

    for (const auto& k : keys)
    {
      const auto& values = std::get<std::vector<int64_t>>(map.at(k).value);
      total = std::accumulate(values.cbegin(), values.cend(), 0);
    }
  }


  // delete keys then add more
  for (int i = 0 ; i < nKeys/10 ; ++i)
    map.erase(std::to_string(i));


  {
    Timer t{"Set 2"};
  
    for (auto i = 0 ; i < nKeys ; ++i)
    {
      auto [it, emplaced] = map.try_emplace(std::to_string(i), std::vector<int64_t> {});

      auto& vec = std::get<std::vector<int64_t>>(it->second.value);
      vec.resize(nValuesPerKey);

      int c = 0;
      std::generate_n(vec.begin(), nValuesPerKey, [&c]{return c++;});
    }
  }
  
  PLOGI << total;    
}


int main (int argc, char ** argv)
{
  plog::init(plog::verbose, &consoleAppender);

  // std::pmr::polymorphic_allocator alloc{fc::MapMemory::getPool(checkMemory)};
  // PmrMap map(alloc);

  // IntVector v{{10,11,12,13}, alloc};
  // StringVector v2{{"long_string_beats_sso_1"}, alloc};

  // map.emplace("k2", v);
  // map.emplace("k3", v2);

  
  perfNormal(100000, 10);
  perfPmr(100000, 10);

  return 0;
}
