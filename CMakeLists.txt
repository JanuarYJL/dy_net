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

# third_party/utility path
set(THIRD_PARTY_PATH ../third_party)
set(UTILITY_PATH ../utility)

# 包含路径
include_directories(
        ${UTILITY_PATH}/include/utility
        ${THIRD_PARTY_PATH}/asio-1.12.2/include
        ${THIRD_PARTY_PATH}/boost_1_66_0/include
        ${THIRD_PARTY_PATH}/protobuf-3.5.1/include
        ${THIRD_PARTY_PATH}/unixODBC-2.3.2/include
        ${THIRD_PARTY_PATH}/jsoncpp-1.8.4/include
        ${THIRD_PARTY_PATH}/libiconv_1_15/include
        /usr/include/oracle/11.2/client64
        ${THIRD_PARTY_PATH}/acl-3.5.1-8/lib_acl/include
        ${THIRD_PARTY_PATH}/acl-3.5.1-8/lib_acl_cpp/include
        ${THIRD_PARTY_PATH}/acl-3.5.1-8/lib_protocol/include
)

link_directories(
        ${UTILITY_PATH}/lib
        ${THIRD_PARTY_PATH}/boost_1_66_0/lib
        ${THIRD_PARTY_PATH}/protobuf-3.5.1/lib
        ${THIRD_PARTY_PATH}/unixODBC-2.3.2/lib
        ${THIRD_PARTY_PATH}/jsoncpp-1.8.4/lib
        ${THIRD_PARTY_PATH}/libiconv_1_15/lib
        /usr/lib/oracle/11.2/client64/lib/
        ${THIRD_PARTY_PATH}/acl-3.5.1-8
)

# out dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/lib/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release)

# sub dir
add_subdirectory(test_util)
add_subdirectory(test_udp)
add_subdirectory(test_cli)
add_subdirectory(capability_test)