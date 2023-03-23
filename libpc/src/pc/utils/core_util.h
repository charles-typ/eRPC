//
// Created by yupeng on 1/18/23.
//

#ifndef PC_LIBPC_SRC_PC_UTILS_UTIL_H_
#define PC_LIBPC_SRC_PC_UTILS_UTIL_H_

#include <complex>
#include <cstddef>
#include <cstdint>
#include <fstream> // std::ifstream
#include <random>
#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <mutex>
#include <iterator>
#include <algorithm>
#include <thread>
#include <numa.h>
#include <unistd.h>

#include "pc/config.h"

namespace pc
{
  namespace utils
  {

    struct CoreInfo
    {
      CoreInfo(size_t id, size_t mapped, size_t req)
          : thread_id_(id),
            requested_core_(req),
            mapped_core_(mapped) {}

      size_t thread_id_;
      size_t requested_core_;
      size_t mapped_core_;

      bool operator<(const CoreInfo &comp) const
      {
        return std::tie(mapped_core_, requested_core_, thread_id_) <
               std::tie(comp.mapped_core_, comp.requested_core_, comp.thread_id_);
      }
      bool operator>(const CoreInfo &comp) const
      {
        return std::tie(mapped_core_, requested_core_, thread_id_) >
               std::tie(comp.mapped_core_, comp.requested_core_, comp.thread_id_);
      }
    };

    static std::vector<size_t> cpu_layout;
    static bool cpu_layout_initialized = false;
    static std::mutex pin_core_mutex;

    /* Keep list of core-thread relationship*/
    static std::list<CoreInfo> core_list;

    static size_t GetCoreId(size_t core)
    {
      size_t result;
      if (cpu_layout_initialized)
      {
        result = cpu_layout.at(core % cpu_layout.size());
      }
      else
      {
        result = core;
      }
      return result;
    }

    /* Print out summary of core-thread relationship */
    static void PrintCoreList(const std::list<CoreInfo> &clist)
    {
      int numa_max_cpus = numa_num_configured_cpus();
      int system_cpus = sysconf(_SC_NPROCESSORS_ONLN);
      std::printf("=================================\n");
      std::printf("          CORE LIST SUMMARY      \n");
      std::printf("=================================\n");
      std::printf("Total Number of Cores: %d : %d \n", numa_max_cpus, system_cpus);
      for (const auto &iter : clist)
      {
        std::printf(
            "|| Core ID: %2zu || Requested: %2zu || "
            "ThreadId: %zu \n",
            iter.mapped_core_, iter.requested_core_, iter.thread_id_);
      }
      std::printf("=================================\n");
    }

    static void PrintBitmask(const struct bitmask *bm)
    {
      for (size_t i = 0; i < bm->size; ++i)
      {
        std::printf("%d", numa_bitmask_isbitset(bm, i));
      }
    }

    static void PrintCoreAssignmentSummary() { PrintCoreList(core_list); }

    static void SetCpuLayoutOnNumaNodes(bool verbose,
                                 const std::vector<size_t> &cores_to_exclude)
    {
      if (cpu_layout_initialized == false)
      {
        int lib_accessable = numa_available();
        if (lib_accessable == -1)
        {
          throw std::runtime_error("libnuma not accessable");
        }
        int numa_max_cpus = numa_num_configured_cpus();
        std::printf("System CPU count %d\n", numa_max_cpus);

        bitmask *bm = numa_bitmask_alloc(static_cast<unsigned int>(numa_max_cpus));
        for (int i = 0; i <= numa_max_node(); ++i)
        {
          numa_node_to_cpus(i, bm);
          if (verbose)
          {
            std::printf("NUMA node %d ", i);
            PrintBitmask(bm);
            std::printf(" CPUs: ");
          }
          for (size_t j = 0; j < bm->size; j++)
          {
            if (numa_bitmask_isbitset(bm, j) != 0)
            {
              if (verbose)
              {
                std::printf("%zu ", j);
              }
              // If core id is not in the excluded list
              if (std::find(cores_to_exclude.begin(), cores_to_exclude.end(), j) ==
                  cores_to_exclude.end())
              {
                cpu_layout.emplace_back(j);
              }
            }
          }
          if (verbose)
          {
            std::printf("\n");
          }
        }
        std::printf("Usable Cpu count %zu\n", cpu_layout.size());

        numa_bitmask_free(bm);
        cpu_layout_initialized = true;
      }
    }

    static size_t GetPhysicalCoreId(size_t core_id)
    {
      size_t core;
      if (cpu_layout_initialized)
      {
        core = cpu_layout.at(core_id);
      }
      else
      {
        unsigned int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
        if ((num_cores > 0) && (core_id >= static_cast<size_t>(num_cores)))
        {
          core = (core_id % num_cores) + 1;
        }
        else
        {
          core = core_id;
        }
      }
      return core;
    }

    static int PinToCore(size_t core_id)
    {
      int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
      if (static_cast<int>(core_id) >= num_cores)
      {
        return -1;
      }

      cpu_set_t cpuset;
      CPU_ZERO(&cpuset);
      CPU_SET(core_id, &cpuset);

      pthread_t current_thread = pthread_self();
      return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    }

  }
}

#endif // PC_LIBPC_SRC_PC_UTILS_UTIL_H_
