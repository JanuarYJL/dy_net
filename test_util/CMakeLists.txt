# 屏蔽#pragma region/endregion报警
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

# define
add_definitions(
        -DUSE_BOOST_LOGGER
)

#src files
aux_source_directory(. TEST_SRC_FILES)

#执行程序
add_executable(test_util.lx ${TEST_SRC_FILES})

#链接使用的库
target_link_libraries(
    test_util.lx
        #内部库
        utility
        #三方库
        boost_log
        boost_log_setup
        boost_system
        boost_filesystem
        boost_serialization
        boost_thread
        boost_locale
        boost_coroutine
        #系统/平台库
        pthread
)