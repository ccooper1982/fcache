
#define FC_DEBUG  // to enable debug from Memory.hpp

#include <fc/Memory.hpp>
#include <fc/LogFormatter.hpp>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>


static plog::ColorConsoleAppender<fc::FcFormatter> consoleAppender;


/*
The first alloc is 528 bytes, which is likely the pool's internal containers,
used to track the pools.
*/

int main (int argc, char ** argv)
{
  plog::init(plog::verbose, &consoleAppender);


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

  PLOGE << "Blob";
  {
    PLOGE << "Creating";
    std::pmr::vector<uint8_t> v(fc::Memory::getPool());
    
    PLOGE << "Inserting";
    v.insert(v.cbegin(), 11'030, 0);

    PLOGE << "Destroying";
  }

  PLOGD << "Allocated: " << fc::Memory::nAllocated() << ", Deallocated: " << fc::Memory::nDeallocated();
}
