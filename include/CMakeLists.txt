include(FetchContent)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
            COMMAND brew --prefix OpenSSL
            RESULT_VARIABLE BREW_OPENSSL
            OUTPUT_VARIABLE BREW_OPENSSL_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_OPENSSL EQUAL 0 AND EXISTS "${BREW_OPENSSL_PREFIX}")
        message(STATUS "Found OpenSSL keg installed by Homebrew at ${BREW_OPENSSL_PREFIX}")
        set(OPENSSL_ROOT_DIR "${BREW_OPENSSL_PREFIX}/")
        set(OPENSSL_INCLUDE_DIR "${BREW_OPENSSL_PREFIX}/include")
        set(OPENSSL_LIBRARIES "${BREW_OPENSSL_PREFIX}/lib")
        set(OPENSSL_CRYPTO_LIBRARY "${BREW_OPENSSL_PREFIX}/lib/libcrypto.dylib")
    endif()
endif()


FIND_PACKAGE( OpenSSL REQUIRED)
set(CPR_FORCE_OPENSSL_BACKEND ON)
set(CPR_FORCE_USE_SYSTEM_CURL ON)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG beb9e98806bb84bcc130a2cebfbcbbc6ce62b335) # the commit hash for 1.7.2
FetchContent_MakeAvailable(cpr)
add_subdirectory(fort)
add_subdirectory(indicators)
#add_subdirectory(binapi)
