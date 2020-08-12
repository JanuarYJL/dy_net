#include <iostream>
#include "net/acceptor.h"
#include "net/connector.h"

using namespace dy;

net::tcp_session::sessionid_type global_sessionid_ = 0;
std::map<net::tcp_session::sessionid_type, std::shared_ptr<net::tcp_session>> session_map_;

std::tuple<net::tcp_session::parse_type, net::buffer::size_type, int> tcp_pack_parse(const net::buffer& buf);
void on_receive(const net::tcp_session::sessionid_type& session_id, const int& pack_type, const char* data_buff, const net::buffer::size_type& length);
void on_disconnect(const net::tcp_session::sessionid_type& session_id, const int& reason_code, const std::string& message);
void on_accept(net::acceptor::socket_type socket);
void logger(const int& type, const char* message);

char data[1024] = {0};

int main()
{
    std::cout << "start ... ..." << std::endl;
    net::asio::io_context ioc_;

    {
        net::tcp_socket ts(ioc_);;
        auto ts_ptr = std::make_shared<net::tcp_session>(std::move(ts), tcp_pack_parse, on_receive, on_disconnect);

        net::udp_socket us(ioc_);;
        auto us_ptr = std::make_shared<net::udp_session>(std::move(us), tcp_pack_parse, on_receive, on_disconnect);
    }

    net::acceptor acceptor_(ioc_, "0.0.0.0", "9797", on_accept);
    acceptor_.start();

    std::vector<std::thread> thrds;
    for (size_t i = 0; i < 2; i++)
    {
        thrds.emplace_back([&ioc_]() {
            ioc_.run();
        });
    }

    ioc_.run();
    std::cout << "over ... ..." << std::endl;
    return 0;
}

std::tuple<net::tcp_session::parse_type, net::buffer::size_type, int> tcp_pack_parse(const net::buffer& buf)
{   
    using parse_type = net::tcp_session::parse_type;
    using size_type = net::buffer::size_type;
    const size_type head_size = 1;
    if (buf.size() >= head_size)
    {
        char len_char = *buf.data();
        if (std::isdigit(len_char))
        {
            size_type need_len = len_char - '0' + head_size;
            if (buf.size() >= need_len)
            {
                return std::make_tuple(parse_type::good, need_len, 0);
            }
            else
            {
                return std::make_tuple(parse_type::less, 0, 0);
            }
        }
        else
        {
            return std::make_tuple(parse_type::bad, 0, 0);
        }
    }
    else
    {
        return std::make_tuple(parse_type::less, 0, 0);
    }
}

void on_receive(const net::tcp_session::sessionid_type& session_id, const int& pack_type, const char* data_buff, const net::buffer::size_type& length)
{
    for (net::buffer::size_type i = 0; i < length; ++i)
    {
        std::cout << data_buff[i] << " ";
    }
    std::cout << std::endl;
}

void on_disconnect(const net::tcp_session::sessionid_type& session_id, const int& reason_code, const std::string& message)
{
    session_map_.erase(session_id);
    std::cout << "disconnect sessionid=" << session_id << " reason code=" << reason_code << " message=" << message << std::endl;
    std::cout << "session_map_.size=" << session_map_.size() << std::endl;
}

void logger(const int& type, const char* message)
{
    std::cout << "logger type=" << type << " message=" << message << std::endl;
}

void on_accept(net::acceptor::socket_type socket)
{
    auto session_sptr = std::make_shared<net::tcp_session>(std::move(socket), tcp_pack_parse, on_receive, on_disconnect);
    session_sptr->set_session_id(++global_sessionid_);
    session_map_[session_sptr->session_id()] = session_sptr;
    //session_sptr->set_options(10, 10, 5, "heartbeat");
    //session_sptr->set_options(10, 10, 5, "");
    //session_sptr->set_options(600, 600, 2, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    session_sptr->start();
    for (size_t i = 0; i < 10000; i++)
    {
        session_sptr->async_send("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 62);
    }
    
}