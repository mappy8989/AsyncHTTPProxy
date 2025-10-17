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
    try {
        std::string buf;
        std::string data_buf;

        std::size_t read_bytes_num =
            co_await async_read_until(client_socket, dynamic_buffer(buf), '\n', use_awaitable);

        auto host_port = findHostPort(buf);
        buf.clear();

        boost::asio::ip::tcp::resolver resolver(io_service);
        auto const endpoints = resolver.resolve(host_port.first, host_port.second);
        tcp::socket remote_socket(io_service);
        co_await async_connect(remote_socket, endpoints, use_awaitable);

        read_bytes_num = co_await async_read_until(remote_socket, dynamic_buffer(buf), "\r\n\r\n",
                                                   use_awaitable);
        size_t read_data_buf_size = buf.size() - read_bytes_num;
        auto content_length = findContentLength(buf);

        if (content_length.has_value()) {
            data_buf = buf.substr(read_bytes_num, buf.size() - read_bytes_num);
            data_buf.resize(content_length.value());

            read_bytes_num = co_await boost::asio::async_read(
                remote_socket,
                boost::asio::buffer(data_buf.data() + read_data_buf_size,
                                    content_length.value() - read_data_buf_size),
                use_awaitable);
        }

        co_await async_write(client_socket, boost::asio::buffer(data_buf, data_buf.size()),
                             use_awaitable);
    } catch (const boost::system::system_error &e) {
        std::cerr << "Boost.System error: " << e.what() << "\n";
        client_socket.close();
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown error occurred in session coroutine\n";
    }
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
