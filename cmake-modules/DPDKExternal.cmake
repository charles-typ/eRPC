# DPDK external project
# target:
#  - dpdk_ep
# defines:
#  - DPDK_HOME
#  - DPDK_INCLUDE_DIRS
#  - DPDK_LIBRARIES

set(DPDK_VERSION "20.02")
#set(DPDK_BUILD ON)

if ((DEFINED ENV{RTE_SDK} AND EXISTS $ENV{RTE_SDK}))
    set(DPDK_ROOT "$ENV{RTE_SDK}")
    set(USE_SYSTEM_DPDK ON)
endif ()

find_package(DPDK ${DPDK_VERSION})
if (DPDK_FOUND)
    #set(THRIFT_BUILD OFF)
    add_custom_target(dpdk_ep)
else(DPDK_FOUND)
    message(STATUS "${Red}Could not use system DPDK, will download and build${ColorReset}")
endif (DPDK_FOUND)

include_directories(SYSTEM ${DPDK_INCLUDE_DIRS})
message(STATUS "DPDK include dir: ${DPDK_INCLUDE_DIRS}")
