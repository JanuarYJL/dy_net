# 屏蔽#pragma region/endregion报警
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

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
        boost_log-mt
        boost_log_setup-mt
        boost_system-mt
        boost_filesystem-mt
        boost_serialization-mt
        boost_thread-mt
        boost_locale-mt
        boost_coroutine-mt
        #系统/平台库
        mswsock
        ws2_32
        pthread
)