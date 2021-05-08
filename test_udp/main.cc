//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include "asio.hpp"

using asio::ip::udp;

class server
{
public:
  server(asio::io_context& io_context, short port)
    : socket_(io_context, udp::endpoint(udp::v4(), port))
  {
    socket_.async_receive_from(
        asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred));
  }

  void handle_receive_from(const asio::error_code& error,
      size_t bytes_recvd)
  {
    if (!error && bytes_recvd > 0)
    {
      std::cout << "success sender_endpoint_ = " 
                << sender_endpoint_.address().to_string()
                << " bytes_recvd = "
                << bytes_recvd
                << std::endl;

      socket_.async_send_to(
          asio::buffer(data_, bytes_recvd), sender_endpoint_,
          boost::bind(&server::handle_send_to, this,
            asio::placeholders::error,
            asio::placeholders::bytes_transferred));
    }
    else
    {
        std::cout << "error sender_endpoint_ = "
                  << sender_endpoint_.address().to_string()
                  << " bytes_recvd = "
                  << bytes_recvd
                  << " error = "
                  << error.message()
                  << std::endl;

        socket_.async_receive_from(
            asio::buffer(data_, max_length), sender_endpoint_,
            boost::bind(&server::handle_receive_from, this,
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred));
    }
  }

  void handle_send_to(const asio::error_code& /*error*/,
      size_t /*bytes_sent*/)
  {
    socket_.async_receive_from(
        asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred));
  }

private:
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  enum { max_length = 1024 };
  char data_[max_length];
};

int main(int argc, char* argv[])
{
  try
  {
    asio::io_context io_context;

    using namespace std; // For atoi.
    server s(io_context, atoi("14040"));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
