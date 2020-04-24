cmake_minimum_required(VERSION 2.8)
project(ARTSynchronized)

if( NOT CMAKE_BUILD_TYPE )
    message(STATUS "No build type selected, default to Release")
    set( CMAKE_BUILD_TYPE Release)
else()
    message(STATUS "Build type is set to ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -march=native -g")

include(lib/jevents.cmake)

find_library(JemallocLib jemalloc)
find_library(TbbLib tbb)
find_package (Threads)

include_directories(
  TBB_INCLUDE_DIR
  third_party)


set(ART_FILES OptimisticLockCoupling/Tree.cpp ROWEX/Tree.cpp ART/Tree.cpp)
add_library(ARTSynchronized ${ART_FILES})
target_link_libraries(ARTSynchronized ${TbbLib} ${JemallocLib} ${CMAKE_THREAD_LIBS_INIT})


set(SOURCE_FILES example.cpp)
add_executable(example ${SOURCE_FILES})
target_link_libraries(example ARTSynchronized)


add_executable(str_bench str_bench.cpp)
target_include_directories(str_bench PRIVATE $<BUILD_INTERFACE:${JEVENTS_INCLUDE_DIR}>)
target_link_libraries(str_bench ARTSynchronized tbb ${CMAKE_THREAD_LIBS_INIT} ${JEVENTS_LIBRARY_PATH})
add_dependencies(str_bench jevents)
