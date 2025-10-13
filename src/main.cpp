#include "headers.h"

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <iostream>
#include <print>
#include <string_view>

using boost::asio::async_read_until;
using boost::asio::awaitable;
using boost::asio::buffer;
using boost::asio::co_spawn;
using boost::asio::dynamic_buffer;
using boost::asio::io_service;
using boost::asio::transfer_at_least;
using boost::asio::use_awaitable;
using boost::asio::ip::tcp;
using boost::system::error_code;

constexpr std::string_view delimiter = "\r\n\r\n";

awaitable<void> session(tcp::socket client_socket, io_service &io_service) {
    std::string buf;

    std::size_t n =
        co_await async_read_until(client_socket, dynamic_buffer(buf), '\n', use_awaitable);

    auto host_port = findHostPort(buf);
    buf.erase(0, n);

    boost::asio::ip::tcp::resolver resolver(io_service);
    auto const endpoints = resolver.resolve(host_port.first, host_port.second);
    co_await async_connect(client_socket, endpoints, use_awaitable);
    n = co_await async_read_until(client_socket, dynamic_buffer(buf), '\n', use_awaitable);
    auto content_length = findContentLength(buf);
    (void)content_length;
}

class Server {
public:
    Server(io_service &io_service, short port)
        : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          socket_(io_service) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(socket_, [this](error_code ec) {
            // code here
            if (!ec) {
                co_spawn(acceptor_.get_executor(),
                         session(std::move(this->socket_), this->io_service_),
                         boost::asio::detached);
            } else {
                std::println("Accept failed: {}", ec.message());
            }

            do_accept();
        });
    }

    io_service &io_service_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main(int argc, char *argv[]) {

    std::string st = "GET /path HTTP/1.1\r\n\
Host: example.com:1234\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\n\
Accept: text/html,application/xhtml+xml\r\n\
Cookie: sessionId=abc123\r\n\
Content-Length: 4567\r\n\
Authorization : Bearer token123 ";

    auto pr = findContentLength(st);

    std::println("{}", pr.value());
    try {
        if (argc != 2) {
            std::cerr << "Usage: proxy_server";
            std::cerr << " <listen_port>\n";
            return 1;
        }
        io_service io_service(1);
        Server server(io_service, std::atoi(argv[1]));
        io_service.run();

    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
