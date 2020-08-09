#ifndef DY_NET_SESSION_H
#define DY_NET_SESSION_H

#include <boost/asio.hpp>

#include "common/macro.h"
#include "net/buffer.h"

namespace dy
{
namespace net
{
class session
{
public:
    using asio = boost::asio;

    using sessionid_type  = std::size_t;
    using size_type = std::size_t;
    using mutex_type = std::mutex;
    using condv_type = std::condition_variable;
    using lock_guard_type = std::lock_guard<mutex_type>;
    using timer_type = asio::steady_timer;

    using func_pack_parse_type = std::function<std::tuple<buffer::size_type /*pack length*/, int /*pack type*/>(const buffer&)>;
    using func_receive_cb_type = std::function<void(const sessionid_type& /*session id*/, const char* /*data buff*/, const buffer::size_type& /*length*/)>;
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

    const std::string local_endpoint() const = 0;

    const std::string remote_endpoint() const = 0;

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
class socket_session : public session, public std::enable_shared_from_this<socket_session<TSocket, TBuffer>>
{
public:
    using socket_type = TSocket;
    using buffer_type = TBuffer;
    using buff_sptr_type = std::shared_ptr<buffer_type>;
    using queue_type = std::queue<buff_sptr_type>;

private:
    mutex_type mutex_;
    socket_type socket_;
    buffer_type recv_buffer_;
    queue_type send_queue_;
    size_type send_queue_capacity_;

public:
    explicit socket_session(socket_type socket,
                            func_pack_parse_type pack_parse_method,
                            func_receive_cb_type receive_callback,
                            func_disconn_cb_type disconnect_callback,
                            const size_type &send_queue_capacity = 0)
        : session(pack_parse_method, receive_callback, disconnect_callback), socket_(std::move(socket)), send_queue_capacity_(send_queue_capacity)
    {
    }

    virtual ~socket_session()
    {

    }

    void set_options( /*发送超时  接收超时  心跳间隔  心跳数据*/ )
    {

    }

    virtual void start() override
    {

    }
    
    virtual void stop() override
    {

    }
    
    virtual bool stopped() override
    {

    }

    const std::string local_endpoint() const override
    {

    }

    const std::string remote_endpoint() const override
    {

    }
};

} // namespace net
} // namespace dy

#endif