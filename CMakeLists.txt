cmake_minimum_required(VERSION 4.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(llsh-installer)

set(CMAKE_CXX_SCAN_FOR_MODULES 0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

option(USE_CCACHE "Use ccache for compilation" OFF)
set(CCACHE_PATH "ccache" CACHE STRING "Path to ccache binary")
if(USE_CCACHE)
    find_program(CCACHE_BINARY ${CCACHE_PATH})
    if(CCACHE_BINARY)
        message(STATUS "[UseCCache] Found ccache at: ${CCACHE_BINARY}")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_BINARY})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_BINARY})
    else()
        message(FATAL_ERROR "[UseCCache] USE_CCACHE enabled, but no "
            "executable found at: ${CCACHE_PATH}")
    endif()
endif()

set(BUILD_SHARED_LIBS OFF)

add_executable(llsh-installer
 main.cpp
 platforms.cpp
 images.cpp
)

target_compile_options(llsh-installer PUBLIC -static)
target_link_options(llsh-installer PUBLIC -static)

include(cmake/CPM.cmake)
set(CPM_SOURCE_CACHE ${CMAKE_BINARY_DIR}/cpmcache)

CPMAddPackage(
  URI "gh:daanx/isocline@1.1.0"
  PATCHES "isocline.patch"
  OPTIONS "IC_BUILD_TESTS OFF"
)
CPMAddPackage(
  URI "gh:libarchive/libarchive@3.8.7"
  OPTIONS "BUILD_SHARED_LIBS OFF"
          "HAVE_SYS_WAIT_H ON"
          "HAVE_WCHAR_H ON"
          "HAVE_SYS_TIME_H ON"
          "HAVE_SYS_STATVFS_H ON"
          "ENABLE_ZSTD OFF"
          "ENABLE_OPENSSL OFF"
)
CPMAddPackage(
  URI "gh:nlohmann/json@3.12.0"
)
CPMAddPackage(
  URI "gh:curl/curl#curl-8_20_0"
  OPTIONS "BUILD_STATIC_LIBS ON"
          "BUILD_SHARED_LIBS OFF"
          "CURL_USE_OPENSSL OFF"
          "CURL_USE_WOLFSSL OFF"
          "CURL_USE_BEARSSL OFF"
          "CURL_USE_MBEDTLS ON"
          "CURL_USE_LIBPSL OFF"
          "CURL_USE_LIBSSH2 OFF"
          "CURL_ZSTD OFF"
          "CURL_BROTLI OFF"
          "USE_NGHTTP2 OFF"
          "USE_LIBIDN2 OFF"
)

target_link_libraries(llsh-installer isocline CURL::libcurl_static nlohmann_json::nlohmann_json archive_static)

add_custom_command(TARGET llsh-installer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the base directory")
