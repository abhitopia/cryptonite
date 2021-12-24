include(FetchContent)

set(CPR_FORCE_OPENSSL_BACKEND ON)
set(CPR_FORCE_USE_SYSTEM_CURL ON)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG beb9e98806bb84bcc130a2cebfbcbbc6ce62b335) # the commit hash for 1.7.2
FetchContent_MakeAvailable(cpr)

add_subdirectory(fort)
add_subdirectory(indicators)

