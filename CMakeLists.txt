cmake_minimum_required(VERSION 3.20)

## --- Fetch FTXUI --------------------------------------------------------------
#include(FetchContent)
#
#set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
#FetchContent_Declare(ftxui
#        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
#        GIT_TAG v0.11
#        )
#
#FetchContent_GetProperties(ftxui)
#if(NOT ftxui_POPULATED)
#    FetchContent_Populate(ftxui)
#    add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()
#
## ------------------------------------------------------------------------------

project(cryptonite LANGUAGES CXX)

add_subdirectory(include)
set(CMAKE_CXX_STANDARD 17)


set(CMAKE_C_COMPILER "gcc-11")
set(CMAKE_CXX_COMPILER "g++-11")

find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -lpthread -std=c++17 -std=gnu++17")


file(GLOB all_includes
        "include/*.h"
        "include/*.hpp"
        "src/*.h"
        "src/*.cpp"
        "cli/*.cpp"
        "cli/*.h"
        "*.cpp"
        )

add_executable(cryptonite ${all_includes})
target_link_libraries(cryptonite CIndicators fort cpr::cpr)

#target_link_libraries(cryptonite PUBLIC CIndicators
#        PRIVATE ftxui::screen
#        PRIVATE ftxui::dom
#        PRIVATE ftxui::component # Not needed for this example.
#        )
target_compile_features(cryptonite PRIVATE cxx_std_17)



