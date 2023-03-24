#!/bin/bash

# https://stackoverflow.com/questions/61286774/how-to-set-specific-cpu-frequency-when-using-intel-pstate

UPPER=1000MHz
LOWER=1000MHz
echo passive | sudo tee /sys/devices/system/cpu/intel_pstate/status
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yupeng/linux-4.19.270/tools/power/cpupower /home/yupeng/linux-4.19.270/tools/power/cpupower/cpupower frequency-info
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yupeng/linux-4.19.270/tools/power/cpupower /home/yupeng/linux-4.19.270/tools/power/cpupower/cpupower idle-set -E
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yupeng/linux-4.19.270/tools/power/cpupower /home/yupeng/linux-4.19.270/tools/power/cpupower/cpupower frequency-set -g userspace
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yupeng/linux-4.19.270/tools/power/cpupower /home/yupeng/linux-4.19.270/tools/power/cpupower/cpupower frequency-set -u $UPPER
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yupeng/linux-4.19.270/tools/power/cpupower /home/yupeng/linux-4.19.270/tools/power/cpupower/cpupower frequency-set -d $UPPER
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yupeng/linux-4.19.270/tools/power/cpupower /home/yupeng/linux-4.19.270/tools/power/cpupower/cpupower frequency-set -f $UPPER
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/yupeng/linux-4.19.270/tools/power/cpupower /home/yupeng/linux-4.19.270/tools/power/cpupower/cpupower frequency-info
#echo
#sudo turbostat --num_iterations 1 --interval 2 2>/dev/shm/turbostat.stderr
#echo
#grep -i "cpufreq" /dev/shm/turbostat.stderr
#echo
#grep -i "frequency" /dev/shm/turbostat.stderr
#echo
#grep -i "max turbo" /dev/shm/turbostat.stderr
#echo
#inxi -C | cat
#echo
#grep "cpu MHz" /proc/cpuinfo
#sleep 3
