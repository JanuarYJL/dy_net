#ifndef DY_COMMON_ERROR_H
#define DY_COMMON_ERROR_H

namespace dy
{
namespace common
{
enum error_code
{
    __error_origin__ = -65536,
    error_queue_full,           // 队列已满
    error_packet_less,          // 包不全
    error_packet_bad,           // 包损坏
    
    error_session_full,         // 会话已满
    error_session_stopped,      // 连接已关闭
    error_session_not_exist,    // 连接不存在

    success_code = 0,           // 成功
};
}//namespace common
}//namespace g6

#endif