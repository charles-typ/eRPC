add_executable(hashtable hashtable/hashtable.cc ${APP_ADDITIONAL_SOURCES})
target_include_directories(hashtable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libpc/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/inih
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/concurrentqueue)
add_dependencies(hashtable dpdk_ep)
target_link_libraries(hashtable ${DPDK_LIBRARIES} erpc)
target_compile_features(hashtable PRIVATE cxx_std_17)

if(PERF)
  target_compile_options(hashtable PRIVATE "-flto")
endif()
target_link_libraries(hashtable ${LIBRARIES})
target_link_libraries(hashtable pc)

add_executable(btree-scan btree-scan/btree-scan.cc ${APP_ADDITIONAL_SOURCES})
target_include_directories(btree-scan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libpc/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/inih
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/concurrentqueue)
add_dependencies(btree-scan dpdk_ep)
target_link_libraries(btree-scan ${DPDK_LIBRARIES} erpc)
target_compile_features(btree-scan PRIVATE cxx_std_17)

if(PERF)
  target_compile_options(btree-scan PRIVATE "-flto")
endif()
target_link_libraries(btree-scan ${LIBRARIES})
target_link_libraries(btree-scan pc)

add_executable(btree-agg btree-agg/btree-agg.cc ${APP_ADDITIONAL_SOURCES})
target_include_directories(btree-agg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../libpc/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/inih
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/concurrentqueue)
add_dependencies(btree-agg dpdk_ep)
target_link_libraries(btree-agg ${DPDK_LIBRARIES} erpc)
target_compile_features(btree-agg PRIVATE cxx_std_17)

if(PERF)
  target_compile_options(btree-agg PRIVATE "-flto")
endif()
target_link_libraries(btree-agg ${LIBRARIES})
target_link_libraries(btree-agg pc)