#ifndef PC_LIBPC_SRC_PC_UTILS_TIME_UTIL_H_
#define PC_LIBPC_SRC_PC_UTILS_TIME_UTIL_H_

#include <time.h>
#include <ctime>
#include <iostream>
#include <chrono>


namespace pc
{
  namespace utils
  {

    //static struct timespec init_time;
    static std::chrono::time_point<std::chrono::high_resolution_clock> init_time;
    inline void init() {
      //init_time = std::chrono::high_resolution_clock::now();
    }

    //inline void init()
    //{
    //  clock_gettime(CLOCK_REALTIME, &init_time);
    //}

    /*
     * initial time that is used to avoid long overflow
     * return the current time in nanoseconds
     */
    inline uint64_t get_time() {
      auto finish = std::chrono::high_resolution_clock::now();
      return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(finish-init_time).count());
    }
    //inline long get_time()
    //{
    //  struct timespec start;
    //  clock_gettime(CLOCK_REALTIME, &start);
    //  std::cout << "Init time is: " << init_time.tv_sec << std::endl;
    //  std::cout << "Current time is: " << start.tv_sec << std::endl;
    //  return (start.tv_sec - init_time.tv_sec) * 1000l * 1000 * 1000 + (start.tv_nsec - init_time.tv_nsec);
    //}
    inline static std::string current_date_time()
    {
      std::time_t rawtime;
      std::tm *timeinfo;
      char buffer[100];

      std::time(&rawtime);
      timeinfo = std::localtime(&rawtime);
      std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", timeinfo);
      return std::string(buffer);
    }

  }

}

#endif // PC_LIBPC_SRC_PC_UTILS_TIME_UTIL_H_