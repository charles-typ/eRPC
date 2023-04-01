if (NOT APPLE AND UNIX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -ldl")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libpc/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/inih
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/concurrentqueue)


if (USE_SYSTEM_DPDK)
        # Parser bench
        add_executable(parser_bench src/parser_benchmark.cpp)
        add_dependencies(parser_bench dpdk_ep)

        target_link_libraries(parser_bench pc)
else()
        target_link_libraries(parser_bench pc_parser)
endif()


install(TARGETS parser_bench
        RUNTIME DESTINATION bin)

# RPC

if (USE_SYSTEM_DPDK)
        add_executable(btree_scan_local_bench src/btree_scan_local_benchmark.cpp)

        add_dependencies(btree_scan_local_bench dpdk_ep)

        target_link_libraries(btree_scan_local_bench pc)

        install(TARGETS btree_scan_local_bench
                RUNTIME DESTINATION bin)

        add_executable(btree_agg_local_bench src/btree_agg_local_benchmark.cpp)

        add_dependencies(btree_agg_local_bench dpdk_ep)

        target_link_libraries(btree_agg_local_bench pc)

        install(TARGETS btree_agg_local_bench
                RUNTIME DESTINATION bin)

        add_executable(hashtable_local_bench src/hashtable_local_benchmark.cpp)

        add_dependencies(hashtable_local_bench dpdk_ep)

        target_link_libraries(hashtable_local_bench pc)

        install(TARGETS hashtable_local_bench
                RUNTIME DESTINATION bin)

else()
        add_library(mind OBJECT
                src/mind_malloc.cpp
                src/mind_malloc.h)

        # MIND bench
        add_executable(mind_hashtable_bench src/hashtable_mind_benchmark.cpp)

        #add_dependencies(mind_hashtable_bench dpdk_ep)

        target_link_libraries(mind_hashtable_bench pc_parser mind pthread)

        install(TARGETS mind_hashtable_bench
                RUNTIME DESTINATION bin)

        add_executable(mind_btree_scan_bench src/btree_scan_mind_benchmark.cpp)

        #add_dependencies(mind_btree_scan_bench dpdk_ep)

        target_link_libraries(mind_btree_scan_bench pc_parser mind pthread)

        install(TARGETS mind_btree_scan_bench
                RUNTIME DESTINATION bin)

        add_executable(mind_btree_agg_bench src/btree_agg_mind_benchmark.cpp)

        #add_dependencies(mind_btree_agg_bench dpdk_ep)

        target_link_libraries(mind_btree_agg_bench pc_parser mind pthread)

        install(TARGETS mind_btree_agg_bench
                RUNTIME DESTINATION bin)


endif()
