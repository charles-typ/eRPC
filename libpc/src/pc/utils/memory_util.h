#ifndef PC_LIBPC_SRC_PC_UTILS_MEMORY_UTIL_H_
#define PC_LIBPC_SRC_PC_UTILS_MEMORY_UTIL_H_

#include "sys/types.h"
#include "sys/sysinfo.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
namespace pc
{
    namespace utils
    {
        static struct sysinfo memInfo;
        static inline void collect_memory_stat()
        {
            sysinfo(&memInfo);
        }
        static inline long long virtual_memory_used()
        {
            long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
            // Add other values in next statement to avoid int overflow on right hand side...
            virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
            virtualMemUsed *= memInfo.mem_unit;
            return virtualMemUsed;
        }
        static inline int virtual_parseLine(char *line)
        {
            // This assumes that a digit will be found and the line ends in " Kb".
            int i = strlen(line);
            const char *p = line;
            while (*p < '0' || *p > '9')
                p++;
            line[i - 3] = '\0';
            i = atoi(p);
            return i;
        }

        static inline long long virtual_memory_used_process()
        {
            FILE *file = fopen("/proc/self/status", "r");
            int result = -1;
            char line[128];

            while (fgets(line, 128, file) != NULL)
            {
                if (strncmp(line, "VmSize:", 7) == 0)
                {
                    result = virtual_parseLine(line);
                    break;
                }
            }
            fclose(file);
            return result;
        }
        static inline long long physical_memory_used()
        {
            long long physMemUsed = memInfo.totalram - memInfo.freeram;
            // Multiply in next statement to avoid int overflow on right hand side...
            physMemUsed *= memInfo.mem_unit;
            return physMemUsed;
        }
        int physical_parseLine(char *line)
        {
            // This assumes that a digit will be found and the line ends in " Kb".
            int i = strlen(line);
            const char *p = line;
            while (*p < '0' || *p > '9')
                p++;
            line[i - 3] = '\0';
            i = atoi(p);
            return i;
        }
        static inline long long physical_memory_used_process()
        {
            FILE *file = fopen("/proc/self/status", "r");
            int result = -1;
            char line[128];

            while (fgets(line, 128, file) != NULL)
            {
                if (strncmp(line, "VmSize:", 7) == 0)
                {
                    result = physical_parseLine(line);
                    break;
                }
            }
            fclose(file);
            return result;
        }
    }

}

#endif // PC_LIBPC_SRC_PC_UTILS_MEMORY_UTIL_H_