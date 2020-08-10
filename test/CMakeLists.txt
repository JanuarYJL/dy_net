# 包含路径
include_directories(
        ../include/dy
)

link_directories(
    ${THIRD_PARTY_PATH}/boost_1_69_0/lib
    ../lib/debug
)

#src files
aux_source_directory(. TEST_SRC_FILES)

#执行程序
add_executable(test.lx ${TEST_SRC_FILES})

#链接使用的库
target_link_libraries(
    test.lx
        #内部库
        dy_net
        #三方库
        boost_log
        boost_log_setup
        boost_filesystem
        boost_serialization
        boost_thread
        boost_locale
        boost_coroutine
        #系统/平台库
        pthread
)