set(FORT_ENABLE_TESTING OFF CACHE INTERNAL "")
project(fort LANGUAGES C)
add_library(fort SHARED fort.c)