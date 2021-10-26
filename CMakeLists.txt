cmake_minimum_required(VERSION 3.20)
project(cryptonite LANGUAGES CXX)

add_subdirectory(lib)
set(CMAKE_CXX_STANDARD 11)


set(CMAKE_C_COMPILER "gcc-11")
set(CMAKE_CXX_COMPILER "g++-11")

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -lpthread")


#add_subdirectory(lib)
#add_subdirectory(lib/indicators)

add_executable(cryptonite main.cpp dataset.cpp dataset.h lib/csv/csv.h lib/date/date.h lib/cli/CLI11.hpp)



