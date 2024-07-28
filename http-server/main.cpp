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
#include "Response.hpp"

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
        // read the http headers, store them in a hashmap
        std::unordered_map<std::string, std::string> http_headers;
        boost::asio::streambuf buffer;
        std::size_t bytes_read = co_await async_read_until(socket, buffer, "\r\n\r\n", boost::asio::use_awaitable);
        std::istream request_stream(&buffer);
        std::string req_line;
        std::getline(request_stream, req_line);
        // create istringstream to parse the first received line (contains http method, version and path)
        std::istringstream header_stream(req_line);
        std::string method, uri, http_version;
        // parse the details
        header_stream >> method >> uri >> http_version;
        // debug: print method, uri and http version
        std::cout << "Method: " << method << std::endl;
        std::cout << "URI: " << uri << std::endl;
        std::cout << "HTTP Version: " << http_version << std::endl;
        // start parsing the http headers sent from client
        std::string header_line;
        while(std::getline(request_stream, header_line) && header_line != "r"){
            auto colon_pos = header_line.find(':');
            // check if current line is valid
            if (colon_pos != std::string::npos) {
                // isolate name and value
                std::string name = header_line.substr(0, colon_pos);
                std::string value = header_line.substr(colon_pos + 1);
                // remove leading and trailing space, so the stored header is valid!
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                http_headers[name] = value;
            }
        }
        // debug: print the header map
        for (const auto& header : http_headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
        // create an instance of Response class
        std::unique_ptr<Response> resp_ptr = std::make_unique<Response>();
        resp_ptr->set_status_message("OK");
        resp_ptr->set_status_code(200);
        resp_ptr->set_header("Content-Type","text/html");
        std::shared_ptr<std::string> body = std::make_shared<std::string>(
                "<title>HTTP-Server Test</title>"
                "<header>Test!</header>"
                "<div><h1>HTTP Server working!</h1></div>"
                );
        resp_ptr->set_raw_body(body);
        // craft the response!
        std::unique_ptr<std::string> finalR = resp_ptr->get_response_string();
        // send the response back to server
        try {
            std::size_t bytes_transferred = co_await boost::asio::async_write(socket,boost::asio::buffer(*finalR));
            std::cout << "Sent " << bytes_transferred << " bytes\n";
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        // read the requested file from the disk, append as body to response
        socket.close();
    }
    catch (std::exception& e)
    {
        std::printf("Exception: %s\n", e.what());
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
