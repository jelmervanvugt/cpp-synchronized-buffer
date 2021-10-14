project(prime)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -g")

find_package(Threads)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
