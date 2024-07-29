//
// Created by User on 29/07/2024.
//

#ifndef HTTP_SERVER_SERVERSTATS_HPP
#define HTTP_SERVER_SERVERSTATS_HPP


#include <ctime>
#include <cstdint>
#include <string>

class ServerStats {
private:
    std::string start_time;
    std::time_t start_time_c;
    uint64_t total_requests;
    uint16_t port;
    std::string bind_on;
public:
    ServerStats();
    ~ServerStats();
    std::string get_start_time();
    static std::string get_server_time();
    uint64_t get_total_requests();
    void set_port(uint16_t port);
    uint16_t get_port();
    void set_bind_on(const std::string& bind_on);
    std::string get_bind_on();
    std::string get_uptime();
    void increase_total_requests();
    std::string get_stats_as_json();
};


#endif //HTTP_SERVER_SERVERSTATS_HPP
