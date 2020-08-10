# CMake 3.8
cmake_minimum_required(VERSION 3.8)

# C++ Pro
project(dy_net CXX)

# C++11 Standard
set(CMAKE_CXX_STANDARD 11)

# CMAKE_C_FLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-deprecated -Wno-conversion-null -Wno-unused-function -Wl,-rpath,./:./depso")
# CMAKE_CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-deprecated -Wno-conversion-null -Wno-unused-function -Wl,-rpath,./:./depso")

# MBCS-D_MBCS -DMBCS
# Unicode -D_UNICODE -DUNICODE
# ASIO std only -DASIO_STANDALONE
add_definitions(
        -D_MBCS -DMBCS
        -DBOOST_ALL_DYN_LINK
)

# third_party
set(THIRD_PARTY_PATH /home/yangjl/workspace/third_party)

# 包含路径
include_directories(
        include/dy
        ${THIRD_PARTY_PATH}/boost_1_69_0/include
)

# src
aux_source_directory(src DY_NET_SRC_FILES)

# out dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release)

# library
add_library(dy_net ${DY_NET_SRC_FILES})


add_subdirectory(test)
