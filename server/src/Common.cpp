#include <fc/Common.hpp>

namespace fc
{
  bool setThreadAffinity(const std::thread::native_handle_type handle, const size_t core)
  {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;
  }
}