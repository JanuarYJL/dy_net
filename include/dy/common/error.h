#ifndef DY_COMMON_ERROR_H
#define DY_COMMON_ERROR_H

namespace dy
{
namespace common
{
enum error_code : int
{
    __origin__ = -65536,    // 错误代码定义起始值

    queue_full,             // 队列已满
    queue_empty,            // 队列为空

    packet_less,            // 数据包不全
    packet_error,           // 数据包损坏
    
    session_full,           // 会话已满
    session_stopped,        // 连接已关闭
    session_not_exist,      // 连接不存在

    normal_error = -1,      // 一般错误
    ok = 0,                 // 成功
};
} // namespace common
} // namespace dy

#endif