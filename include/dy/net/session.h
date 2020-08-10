#ifndef DY_NET_SESSION_H
#define DY_NET_SESSION_H

#include "common/error.h"
#include "common/constant.h"
#include "common/macro.h"

#include <queue>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>

#include "net/buffer.h"

namespace dy
{
namespace net
{
namespace asio = boost::asio;
class session : public std::enable_shared_from_this<session>
{
public:
    using sessionid_type  = std::size_t;
    using size_type = std::size_t;
    using mutex_type = std::mutex;
    using condv_type = std::condition_variable;
    using lock_guard_type = std::lock_guard<mutex_type>;
    using timer_type = asio::steady_timer;
    using time_point_type = timer_type::time_point;

    using func_pack_parse_type = std::function<std::tuple<buffer::size_type /*pack length*/, int /*pack type*/>(const buffer&)>;
    using func_receive_cb_type = std::function<void(const sessionid_type& /*session id*/, const int& /*pack type*/, const char* /*data buff*/, const buffer::size_type& /*length*/)>;
    using func_disconn_cb_type = std::function<void(const sessionid_type& /*session id*/, const int& /*reason code*/, const std::string& /*message*/)>;
    using func_log_type        = std::function<void(const int& /*type*/, const char* /*message*/)>;

protected:
    sessionid_type session_id_;

    func_pack_parse_type func_pack_parse_method_;
    func_receive_cb_type func_receive_callback_;
    func_disconn_cb_type func_disconnect_callback_;

public:
    DISABLE_COPY_ASSIGN(session);

    explicit session(func_pack_parse_type pack_parse_method, func_receive_cb_type receive_callback, func_disconn_cb_type disconnect_callback)
        : func_pack_parse_method_(pack_parse_method), func_receive_callback_(receive_callback), func_disconnect_callback_(disconnect_callback)
    {
    }

    virtual ~session();

    virtual void start() = 0;
    
    virtual void stop() = 0;
    
    virtual bool stopped() = 0;

    virtual const std::string local_endpoint() const = 0;

    virtual const std::string remote_endpoint() const = 0;

    const sessionid_type& session_id() const
    {
        return session_id_;
    }

    void set_session_id(const sessionid_type& session_id)
    {
        session_id_ = session_id;
    }
};

template<class TSocket, class TBuffer>
class socket_session : public session
{
public:
    using socket_type = TSocket;
    using buffer_type = TBuffer;
    using buff_sptr_type = std::shared_ptr<buffer_type>;
    using queue_type = std::queue<buff_sptr_type>;

private:
    mutex_type mutex_;              // Mutex
    socket_type socket_;            // Socket
    buffer_type recv_buffer_;       // 接收缓存
    queue_type send_queue_;         // 发送队列
    size_type send_queue_capacity_; // 发送队列容量(上限)
    buff_sptr_type send_buff_ptr_;  // 发送缓存

    size_type send_timeout_;        // 发送超时
    size_type recv_timeout_;        // 接收超时
    size_type heartbeat_interval_;  // 心跳间隔
    std::string heartbeat_data_;    // 心跳数据

    timer_type recv_deadline_;
    timer_type send_deadline_;
    timer_type heartbeat_timer_;
    timer_type non_empty_send_queue_;

public:
    explicit socket_session(socket_type socket,
                            func_pack_parse_type pack_parse_method,
                            func_receive_cb_type receive_callback,
                            func_disconn_cb_type disconnect_callback,
                            const size_type &send_queue_capacity = 0) noexcept
        : session(pack_parse_method, receive_callback, disconnect_callback),
          socket_(std::move(socket)),
          send_queue_capacity_(send_queue_capacity),
          recv_deadline_(socket_.get_io_service()),
          send_deadline_(socket_.get_io_service()),
          heartbeat_timer_(socket_.get_io_service()),
          non_empty_send_queue_(socket_.get_io_service())
    {
        recv_deadline_.expires_at(time_point_type::max());
        send_deadline_.expires_at(time_point_type::max());
        heartbeat_timer_.expires_at(time_point_type::max());
        non_empty_send_queue_.expires_at(time_point_type::max());
    }

    virtual ~socket_session()
    {

    }

    void set_options(const int& send_timeout = 30, const int& recv_timeout = 30, const int& heartbeat_interval = 10, const std::string& heartbeat_data = "")
    {
        send_timeout_ = send_timeout;
        recv_timeout_ = recv_timeout;
        heartbeat_interval_ = heartbeat_interval;
        heartbeat_data_ = heartbeat_data;
    }

    virtual void start() override
    {
        // 启动接收/发送链
        handle_recv();
        recv_deadline_.async_wait(std::bind(&socket_session<TSocket, TBuffer>::check_deadline, std::dynamic_pointer_cast<socket_session<TSocket, TBuffer>>(shared_from_this()), std::ref(recv_deadline_)));
        handle_send();
        send_deadline_.async_wait(std::bind(&socket_session<TSocket, TBuffer>::check_deadline, std::dynamic_pointer_cast<socket_session<TSocket, TBuffer>>(shared_from_this()), std::ref(send_deadline_)));
    }
    
    virtual void stop() override
    {

    }
    
    virtual bool stopped() override
    {
        return !socket_.is_open();
    }

    const std::string local_endpoint() const override
    {
        lock_guard_type lk(mutex_);
        if (socket_.is_open())
        {
            return socket_.local_endpoint().address().to_string();
        }
        else
        {
            return "";
        }
    }

    const std::string remote_endpoint() const override
    {
        lock_guard_type lk(mutex_);
        if (socket_.is_open())
        {
            return socket_.remote_endpoint().address().to_string();
        }
        else
        {
            return "";
        }
    }

private:
    void handle_stop()
    {
        if (func_disconnect_callback_)
        {
            func_disconnect_callback_(session_id(), 0, "");
        }
        lock_guard_type lk(mutex_);
        if (socket_.is_open())
        {
            boost::system::error_code ec;
            socket_.close(ec);
        }
    }

    void handle_recv()
    {
        lock_guard_type lk(mutex_);
        if (stopped())
        {
            return;
        }
        recv_deadline_.expires_after(asio::chrono::seconds(recv_timeout_));
        async_recv();
    }

    void async_recv()
    {
        auto self_ = std::dynamic_pointer_cast<socket_session<TSocket, TBuffer>>(shared_from_this());
        socket_.async_read_some(asio::buffer(recv_buffer_.writable_buff(), recv_buffer_.writable_size()), [this, self_](std::error_code ec, std::size_t bytes_transferred) {
            if (!ec)
            {
                // 更新接收缓存有效长度
                recv_buffer_.set_size(recv_buffer_.size() + bytes_transferred);

                while (true)
                {
                    // 解析接收缓存数据
                    int parse_type = 0;
                    int parse_result = 0;
                    std::tie(parse_result, parse_type) = func_pack_parse_method_(recv_buffer_);
                    if (parse_result == common::error_packet_bad)
                    {
                        // 解包异常 停止
                        handle_stop();
                        break;
                    }
                    else if (parse_result == common::error_packet_less)
                    {
                        // 整理缓存 继续解包
                        recv_buffer_.move_to_head(parse_result);
                        // 不足一包 继续接收
                        handle_recv();
                        break;
                    }
                    else
                    {
                        // 将解析出的包回调给业务层
                        if (func_receive_callback_)
                        {
                            func_receive_callback_(session_id(), parse_type, recv_buffer_.data(), parse_result);
                        }
                        recv_buffer_.pop_cache(parse_result);
                    }
                }
            }
            else
            {
                // 接收异常 停止
                handle_stop();
            }
        });
    }

    void handle_send()
    {
        if (!stopped())
        {
            lock_guard_type lk(mutex_);
            if (send_queue_.empty())
            {
                // 无数据时挂起等待数据
                non_empty_send_queue_.expires_at(time_point_type::max());
                non_empty_send_queue_.async_wait(std::bind(&socket_session<TSocket, TBuffer>::handle_send, std::dynamic_pointer_cast<socket_session<TSocket, TBuffer>>(shared_from_this())));
                // 设置心跳定时器
                heartbeat_timer_.expires_after(asio::chrono::seconds(heartbeat_interval_));
                heartbeat_timer_.async_wait(std::bind(&socket_session<TSocket, TBuffer>::check_heartbeat, std::dynamic_pointer_cast<socket_session<TSocket, TBuffer>>(shared_from_this())));
            }
            else
            {
                // 发送数据
                send_deadline_.expires_after(asio::chrono::seconds(send_timeout_));
                async_send();
            }
        }
    }

    void async_send()
    {
        auto self_ = std::dynamic_pointer_cast<socket_session<TSocket, TBuffer>>(shared_from_this());
        send_buff_ptr_ = send_queue_.front();
        send_queue_.pop();
        asio::async_write(socket_, asio::buffer(send_buff_ptr_->data(), send_buff_ptr_->size()), [this, self_](std::error_code ec, std::size_t bytes_transferred) {
            if (!ec)
            {
                // 继续检测 发送
                handle_send();
            }
            else
            {
                // 发送异常 停止
                handle_stop();
            }
        });
    }
};

// explicit class declaration
// TCP
using tcp_socket = dy::net::asio::ip::tcp::socket;
using tcp_session = dy::net::socket_session<tcp_socket, dy::net::buffer>;
// UDP
using udp_socket = dy::net::asio::ip::udp::socket;
using udp_session = dy::net::socket_session<udp_socket, dy::net::buffer>;

} // namespace net
} // namespace dy

#endif