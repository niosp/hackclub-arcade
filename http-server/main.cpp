/*
 *   _    _ _______ _______ _____     _____
 *  | |  | |__   __|__   __|  __ \   / ____|
 *  | |__| |  | |     | |  | |__) | | (___   ___ _ ____   _____ _ __
 *  |  __  |  | |     | |  |  ___/   \___ \ / _ \ '__\ \ / / _ \ '__|
 *  | |  | |  | |     | |  | |       ____) |  __/ |   \ V /  __/ |
 *  |_|  |_|  |_|     |_|  |_|      |_____/ \___|_|    \_/ \___|_|
 *
 */
#include <boost/asio.hpp>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable_t;

using tcp_acceptor = use_awaitable_t<>::as_default_on_t<tcp::acceptor>;
using tcp_socket = use_awaitable_t<>::as_default_on_t<tcp::socket>;

namespace this_coro = boost::asio::this_coro;

awaitable<void> process_client_request(tcp_socket socket){
    try
    {
        // parse the http headers!
        // do post_processing!
        std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "\r\n"
                "Hello, world!";
        co_await async_write(socket, boost::asio::buffer(response));

        // read the http headers, store them in a hashmap
        std::unordered_map<std::string, std::string> http_headers;
        boost::asio::streambuf buffer;
        std::size_t bytes_read = co_await async_read_until(socket, buffer, "\r\n\r\n", boost::asio::use_awaitable);
        socket.close();
    }
    catch (std::exception& e)
    {
        std::printf("echo Exception: %s\n", e.what());
    }
}

awaitable<void> processor(int port){
    // create the executor
    auto executor = co_await this_coro::executor;
    // create the tcp acceptor that listens on the port (1st func arg)
    tcp_acceptor acceptor(executor, {tcp::v4(), static_cast<boost::asio::ip::port_type>(port)});
    // accept tcp connections from clients
    std::printf("Server running on port %d\n", port);
    for (;;)
    {
        // accept the connection, move the socket inside the coroutine
        auto socket = co_await acceptor.async_accept();
        co_spawn(executor, process_client_request(std::move(socket)), detached);
    }
}
int main(int argc, char* argv[]) {
    // port should be supplied as argument (argv[1])
    if(argc <= 1){
        std::cerr << "Usage: ./<server_binary> <port>\n";
        return 0;
    }
    try{
        // create a boost io context
        boost::asio::io_context io_context(1);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){ io_context.stop(); });
        // spawn a coroutine that handles incoming tcp connections, pass the port
        co_spawn(io_context, processor(std::stoi(argv[1])), detached);
        // run the io_context!
        io_context.run();
    }catch(std::exception& ex){
        // exception will be logged including error message
        std::printf("Exception occured: %s\n", ex.what());
    }
    return 0;
}
