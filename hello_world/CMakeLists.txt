include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/asio/include
        ${DPDK_INCLUDE_DIRS})

add_executable(serverd src/server.cpp)

add_dependencies(serverd dpdk_ep)

target_link_libraries(serverd erpc dl numa mlx5 ibverbs)

set_target_properties( serverd PROPERTIES COMPILE_FLAGS "-std=c++11 -Wno-deprecated-declarations -march=native -DERPC_DPDK=true" )




add_executable(clientd src/client.cpp)

add_dependencies(clientd dpdk_ep)

target_link_libraries(clientd erpc dl numa mlx5 ibverbs)

set_target_properties( clientd PROPERTIES COMPILE_FLAGS "-std=c++11 -Wno-deprecated-declarations -march=native -DERPC_DPDK=true" )