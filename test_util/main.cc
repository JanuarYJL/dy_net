#include <iostream>
#include <set>
#include <string>

#include "boost/algorithm/string.hpp"

#include "daemon/daemon.h"
#include "logger/logger.hpp"
#include "net/acceptor.h"
#include "net/connector.h"
#include "net/session.h"
#include "net/client.h"

#include "version.h"

using namespace g6;
using namespace g6::utility;

void server_on_exit(void)
{
    //do something when process exits
    exit(0);
    return;
}

void signal_crash_handler(int sig)
{
    server_on_exit();
    exit(-1);
}

void signal_exit_handler(int sig)
{
    server_on_exit();
    exit(1);
}

//! net测试相关函数
utility::tcp_session::sessionid_type global_sessionid_ = 0;
std::map<utility::tcp_session::sessionid_type, std::shared_ptr<utility::tcp_session>> session_map_;

std::tuple<utility::tcp_session::parse_type, utility::buffer::size_type, int> tcp_pack_parse(const utility::buffer& buf);
void on_receive(const utility::tcp_session::sessionid_type& session_id, const int& pack_type, const char* data_buff, const utility::buffer::size_type& length);
void on_disconnect(const utility::tcp_session::sessionid_type& session_id, const int& reason_code, const std::string& message);
void on_accept(utility::acceptor::socket_type socket);
void logger(const int& type, const char* message);

//! test global variables
utility::asio::io_context k_ioc;
// const auto k_remote_endpoint_ip = "172.16.9.11";
// const auto k_remote_endpoint_tcp_port = "11101";
// const auto k_remote_endpoint_udp_port = "11102";
const auto k_remote_endpoint_ip = "114.80.94.25";
const auto k_remote_endpoint_tcp_port = "8280";
const auto k_remote_endpoint_udp_port = "8281";
// const auto k_local_endpoint_ip = "0.0.0.0"; // 10.10.10.42
const auto k_local_endpoint_ip = "::"; // ipv6
const auto k_local_endpoint_tcp_port = "14201";
const auto k_local_endpoint_udp_port = "14202";

//! client test
void test_tcp_client();
void test_udp_client();
//! multi session test
void test_tcp_multi_session();
void test_udp_multi_session();
void test_flex_multi_session();

//! temp test
void temp_test()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;    
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_protocol = IPPROTO_IP;          /* Any protocol */
    if (getaddrinfo("::1", "18305", &hints, &result)!= 0)
    {
        std::cout << errno << std::endl;
    }


    for (rp = result; rp != NULL; rp = rp->ai_next)
    {

       sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
       if (sfd == -1)
           continue;

       if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
           break;                  /* Success */

       close(sfd);
     }
     if (rp == NULL)
     {
         std::cout << errno << std::endl;
     }
}

void temp_udp_test()
{
    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(50001);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    char recvline[1024];

    struct sockaddr recvaddr = {0};
    socklen_t recvaddrlen = sizeof(recvaddr);
    recvfrom(sockfd, recvline, 1024, 0, (struct sockaddr *)&recvaddr, &recvaddrlen);

    printf("%s\n", recvline);

    sprintf(recvline, "aassddffgghh!!");
    sendto(sockfd, recvline, strlen(recvline), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));

    int other_sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    sprintf(recvline, "zzxxccvvbbnn!!");
    sendto(other_sockfd, recvline, strlen(recvline), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));

    sprintf(recvline, "112233445566!!");
    sendto(sockfd, recvline, strlen(recvline), 0, (struct sockaddr *)&recvaddr, sizeof(recvaddr));

    close(sockfd);
    close(other_sockfd);
}

int main(int argc, char *argv[])
{
#pragma region 基础设置及初始化
    // 解析程序启动参数
    std::set<std::string> exec_arguments;
    for (int i = 1; i < argc; ++i)
    {
        exec_arguments.emplace(boost::to_lower_copy(std::string(argv[i])));
    }

    // 环境初始化 注意数值类型不要切换避免中文环境下有逗号分隔
    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "");
    std::locale::global(std::locale(std::locale(""), std::locale::classic(), std::locale::numeric));

    atexit(server_on_exit);
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGPIPE, SIG_IGN);              // ignore SIGPIPE
    // signal(SIGBUS, signal_crash_handler);  // 总线错误
    // signal(SIGSEGV, signal_crash_handler); // SIGSEGV，非法内存访问
    // signal(SIGFPE, signal_crash_handler);  // SIGFPE，数学相关的异常，如被0除，浮点溢出，等等
    // signal(SIGABRT, signal_crash_handler); // SIGABRT，由调用abort函数产生，进程非正常退出

    // daemon
    if (exec_arguments.count("-d") > 0 || exec_arguments.count("--daemon") > 0)
    {
        if (Daemon::DaemonModeRun())
        {
            exit(EXIT_FAILURE);
        }
    }
    // repeated start
    if (exec_arguments.count("-rs") == 0 && exec_arguments.count("--repeatedstart") == 0)
    {
        if (Daemon::AlreadyRunning(EXEC_UNIQUE_PID_FILE))
        {
            std::cout << APP_NAME << " is running, if you want to repeated start exec it by '-rs' or '--repeatedstart'." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // 初始化日志环境
    UtilityLogger::InitLoggerEnv("config/log.conf");
    UTILITY_LOGGER(info) << "程序启动 日志初始化完成 ...";
    // 版本信息
    UTILITY_LOGGER(info) << APP_NAME << " " << VERSION_TEXT_COMP << " Build time:" << BUILD_DATE_TIME;

    UTILITY_LOGGER(info) << "case abc 日志...";
    UTILITY_LOGGER(INFO) << "CASE ABC 日志...";
#pragma endregion

    // temp_test();
    //! lib test
    std::cout << "start ... ..." << std::endl;
    // test_tcp_client();
    test_udp_client();
    // test_tcp_multi_session();
    // test_udp_multi_session();
    // test_flex_multi_session();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    k_ioc.run();
    std::cout << "over ... ..." << std::endl;
    return 0;
}

std::tuple<utility::tcp_session::parse_type, utility::buffer::size_type, int> tcp_pack_parse(const utility::buffer& buf)
{   
    using parse_type = utility::tcp_session::parse_type;
    using size_type = utility::buffer::size_type;
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

void on_receive(const utility::tcp_session::sessionid_type& session_id, const int& pack_type, const char* data_buff, const utility::buffer::size_type& length)
{
    for (utility::buffer::size_type i = 0; i < length; ++i)
    {
        std::cout << data_buff[i] << " ";
    }
    std::cout << std::endl;
}

void on_disconnect(const utility::tcp_session::sessionid_type& session_id, const int& reason_code, const std::string& message)
{
    session_map_.erase(session_id);
    std::cout << "disconnect sessionid=" << session_id << " reason code=" << reason_code << " message=" << message << std::endl;
    std::cout << "session_map_.size=" << session_map_.size() << std::endl;
}

void logger(const int& type, const char* message)
{
    std::cout << "logger type=" << type << " message=" << message << std::endl;
}

void on_accept(utility::acceptor::socket_type socket)
{
    auto session_sptr = std::make_shared<utility::tcp_session>(std::move(socket), tcp_pack_parse, on_receive, on_disconnect);
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

void test_tcp_client()
{
    std::thread thrd([]() {
        const size_t thread_num = 3;
        const size_t disconn_thrd_num = 0;
        const size_t close_thrd_num = 0;

        auto tc_ptr = std::make_shared<utility::tcp_client>(k_ioc);
        tc_ptr->set_endpoint(k_remote_endpoint_ip, k_remote_endpoint_tcp_port);
        tc_ptr->set_callback(tcp_pack_parse, on_receive, on_disconnect);
        tc_ptr->set_options("login test by tcp", true, "heart beat by tcp", 2, 8, 0);
        tc_ptr->connect();
        // 发送数据测试
        for (size_t i = 0; i < thread_num - 1; ++i)
        {
            std::thread send_thrd([tc_ptr]() {
                std::string data;
                int interval = 1;
                while (!k_ioc.stopped())
                {
                    // 1~8s间隔递增循环发送数据
                    data.clear();
                    for (int i = 0; i < interval; ++i)
                    {
                        data += i + '0';
                        data += i + '0';
                        data += i + '0';
                    }
                    tc_ptr->async_send(data.c_str(), data.length());
                    std::this_thread::sleep_for(std::chrono::seconds(interval++));
                    if (interval > 8)
                    {
                        interval = 1;
                    }
                }
            });
            send_thrd.detach();
        }
        // 断线重连测试
        for (size_t i = 0; i < disconn_thrd_num; ++i)
        {
            std::thread disconn_thrd([tc_ptr]() {
                while (!k_ioc.stopped())
                {
                    // 20s 断线一次
                    std::this_thread::sleep_for(std::chrono::seconds(12));
                    tc_ptr->disconnect();
                }
            });
            disconn_thrd.detach();
        }
        // 关闭连接测试
        for (size_t i = 0; i < close_thrd_num; ++i)
        {
            std::thread close_thrd([tc_ptr]() {
                while (!k_ioc.stopped())
                {
                    // 2min后关闭连接
                    std::this_thread::sleep_for(std::chrono::minutes(2));
                    tc_ptr->close();
                }
            });
            close_thrd.detach();
        }

        // 3+1 IO工作线程
        for (int i = 0; i < 3; ++i)
        {
            std::thread thrd([]() { k_ioc.run(); });
            thrd.detach();
        }
        k_ioc.run();
    });

    thrd.detach();
}

void test_udp_client()
{
    std::thread thrd([]() {
        const size_t thread_num = 3;
        const size_t disconn_thrd_num = 0;
        const size_t close_thrd_num = 0;

        utility::udp_socket us(k_ioc);
        auto us_ptr = std::make_shared<utility::udp_session>(std::move(us), tcp_pack_parse, on_receive, on_disconnect);

        auto tc_ptr = std::make_shared<utility::tcp_client>(k_ioc);
        auto uc_ptr = std::make_shared<utility::udp_client>(k_ioc);
        uc_ptr->set_endpoint(k_remote_endpoint_ip, k_remote_endpoint_udp_port);
        uc_ptr->set_callback(tcp_pack_parse, on_receive, on_disconnect);
        uc_ptr->set_options("login test by udp", true, "heart beat by udp", 2, 8, 0);
        uc_ptr->connect();
        // 发送数据测试
        for (size_t i = 0; i < thread_num - 1; ++i)
        {
            std::thread send_thrd([uc_ptr]() {
                std::string data;
                int interval = 1;
                while (!k_ioc.stopped())
                {
                    // 1~8s间隔递增循环发送数据
                    data.clear();
                    for (int i = 0; i < interval; ++i)
                    {
                        data += i + '0';
                        data += i + '0';
                        data += i + '0';
                    }
                    uc_ptr->async_send(data.c_str(), data.length());
                    std::this_thread::sleep_for(std::chrono::seconds(interval++));
                    if (interval > 8)
                    {
                        interval = 1;
                    }
                }
            });
            send_thrd.detach();
        }
        // 断线重连测试
        for (size_t i = 0; i < disconn_thrd_num; ++i)
        {
            std::thread disconn_thrd([uc_ptr]() {
                while (!k_ioc.stopped())
                {
                    // 20s 断线一次
                    std::this_thread::sleep_for(std::chrono::seconds(12));
                    uc_ptr->disconnect();
                }
            });
            disconn_thrd.detach();
        }
        // 关闭连接测试
        for (size_t i = 0; i < close_thrd_num; ++i)
        {
            std::thread close_thrd([uc_ptr]() {
                while (!k_ioc.stopped())
                {
                    // 2min后关闭连接
                    std::this_thread::sleep_for(std::chrono::minutes(2));
                    uc_ptr->close();
                }
            });
            close_thrd.detach();
        }

        // 3+1 IO工作线程
        for (int i = 0; i < 3; ++i)
        {
            std::thread thrd([]() { k_ioc.run(); });
            thrd.detach();
        }
        k_ioc.run();
    });
    
    thrd.detach();
}

void test_tcp_multi_session()
{
    std::thread thrd([]() {
        const size_t thread_num = 2;

        auto apt_ptr = std::make_shared<utility::acceptor>(k_ioc, k_local_endpoint_ip, k_local_endpoint_tcp_port, on_accept);
        apt_ptr->start();

        std::vector<std::thread> thrds;
        for (size_t i = 0; i < thread_num - 1; i++)
        {
            thrds.emplace_back([]() {
                k_ioc.run();
            });
        }

        k_ioc.run();
    });

    thrd.detach();
}

void test_udp_multi_session()
{

}

void test_flex_multi_session()
{

}
