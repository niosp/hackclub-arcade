//
// Created by User on 29/07/2024.
//

#include <chrono>
#include "ServerStats.hpp"
#include "lib/json.hpp"
#include "util.hpp"

ServerStats::ServerStats() {
    this->total_requests = 0;
    this->start_time = get_current_timestamp();
    this->start_time_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    this->port = 0;
}

ServerStats::~ServerStats() = default;

std::string ServerStats::get_start_time() {
    return this->start_time;
}

uint64_t ServerStats::get_total_requests() {
    return this->total_requests;
}

void ServerStats::set_port(uint16_t port) {
    this->port = port;
}

uint16_t ServerStats::get_port() {
    return this->port;
}

void ServerStats::set_bind_on(const std::string& bind_on) {
    this->bind_on = bind_on;
}

std::string ServerStats::get_bind_on() {
    return this->bind_on;
}

std::string ServerStats::get_uptime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::time_t diff = now_c - this->start_time_c;
    std::string uptime = std::to_string(diff / 3600) + " hours, " + std::to_string((diff % 3600) / 60) + " minutes, " + std::to_string(diff % 60) + " seconds";
    return uptime;
}

std::string ServerStats::get_server_time() {
    return get_current_timestamp();
}

void ServerStats::increase_total_requests() {
    this->total_requests++;
}

std::string ServerStats::get_stats_as_json() {
    nlohmann::json j;
    j["total_requests"] = this->total_requests;
    j["port"] = this->port;
    j["bind_on"] = this->bind_on;
    j["uptime"] = this->get_uptime();
    j["server_time"] = get_server_time();
    j["start_time"] = this->start_time;
    return j.dump();
}

