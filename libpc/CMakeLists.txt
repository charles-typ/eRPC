
message(STATUS "include dir: ${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/inih
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/concurrentqueue
        ${DPDK_INCLUDE_DIRS})

add_library(pc OBJECT
        src/pc/datastructure/bplustree.h
        src/pc/datastructure/hashtable.h
        src/pc/datastructure/list.h
        src/pc/parser/rpc_parse.cpp
        src/pc/parser/rpc_parse.h
        src/pc/parser/parse.h
        src/pc/utils/logger.cpp
        src/pc/utils/logger.h
        src/pc/worker/btree_agg_worker.cpp
        src/pc/worker/btree_agg_worker.h
        src/pc/worker/btree_scan_worker.cpp
        src/pc/worker/btree_scan_worker.h
        src/pc/worker/ht_worker.cpp
        src/pc/worker/ht_worker.h
        src/pc/worker/rpc_worker.cpp
        src/pc/worker/rpc_worker.h
        src/pc/config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/inih/ini.c)

add_dependencies(pc dpdk_ep)
target_link_libraries(pc ${DPDK_LIBRARIES})
target_link_libraries(pc pthread dl rt m numa mlx5 ibverbs)
set_target_properties(pc PROPERTIES LINKER_LANGUAGE CXX)
# Enable C++ 17 here
# https://stackoverflow.com/questions/45688522/how-to-enable-c17-in-cmake
target_compile_features(pc PRIVATE cxx_std_17)


install(TARGETS pc
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(DIRECTORY src/pc
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

if (BUILD_TESTS)
    include_directories(${CATCH_INCLUDE_DIR})
    add_executable(pc_tests
            test/bplustree_test.cpp
            #test/hashtable_test.cpp
            #test/list_test.cpp
            test/test_util.h)
    target_link_libraries(pc_tests pc)
    add_dependencies(pc_tests catch_ep)
    # Register test
    add_test(PCTest pc_tests)
endif()

#        add_library(pc_parser OBJECT
#                src/pc/datastructure/bplustree.h
#                src/pc/datastructure/hashtable.h
#                src/pc/datastructure/list.h
#                #src/pc/parser/local_parse.h
#                #src/pc/parser/local_parse.cpp
#                #src/pc/parser/pulse_parse.h
#                #src/pc/parser/pulse_parse.cpp
#                src/pc/parser/rpc_parse.cpp
#                src/pc/parser/rpc_parse.h
#                src/pc/parser/parse.h
#                src/pc/utils/rand_util.h
#                src/pc/config.h
#                ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/inih/ini.c)
#        
#        #add_dependencies(pc_parser dpdk_ep)
#        #target_link_libraries(pc ${DPDK_LIBRARIES})
#        #target_link_libraries(pc pthread dl rt m numa mlx5 ibverbs)
#        set_target_properties(pc_parser PROPERTIES LINKER_LANGUAGE CXX)
#        # Enable C++ 17 here
#        # https://stackoverflow.com/questions/45688522/how-to-enable-c17-in-cmake
#        target_compile_features(pc_parser PRIVATE cxx_std_17)
#        
#        
#        install(TARGETS pc_parser
#                RUNTIME DESTINATION bin
#                ARCHIVE DESTINATION lib
#                LIBRARY DESTINATION lib)
#        install(DIRECTORY src/pc
#                DESTINATION include
#                FILES_MATCHING PATTERN "*.h")
#
#endif()