/*
 * Response class for the HTTP server: crafts the response to be sent back to the client
 */

#include "Response.hpp"

#include <utility>
#include <iostream>

Response::Response() {
    this->raw_body = std::make_shared<std::string>();
    this->headers = std::unordered_map<std::string, std::string>();
    this->status_code = 200;
    this->status_message = "OK";
    this->headers["Content-Type"] = "text/plain";
    this->headers["Connection"] = "close";
}

Response::~Response() = default;

std::unique_ptr<std::string> Response::craft_response() {
    std::unique_ptr<std::string> response = std::make_unique<std::string>();
    // append the status line
    *response += "HTTP/1.1 " + std::to_string(this->status_code) + " " + this->status_message + "\r\n";
    // calculate the content length
    this->set_content_length(this->raw_body->size());
    // append the headers
    for (const auto& header : this->headers) {
        *response += header.first + ": " + header.second + "\r\n";
    }
    // append the body
    *response += "\r\n" + *this->raw_body;
    return std::move(response);
}

std::shared_ptr<std::string> Response::get_raw_body() {
    return this->raw_body;
}

void Response::set_raw_body(std::shared_ptr<std::string> p_raw_body) {
    this->raw_body = std::move(p_raw_body);
}

void Response::set_header(const std::string& key, const std::string& value) {
    this->headers[key] = value;
}

const std::string& Response::get_header(const std::string& key) {
    return this->headers[key];
}

void Response::set_status_code(int p_status_code) {
    this->status_code = p_status_code;
}

int Response::get_status_code() const {
    return this->status_code;
}

void Response::set_status_message(std::string p_status_message) {
    this->status_message = std::move(p_status_message);
}

std::string Response::get_status_message() {
    return this->status_message;
}

void Response::set_content_length(int content_length) {
    this->headers["Content-Length"] = std::to_string(content_length);
}

int Response::get_content_length() {
    return std::stoi(this->headers["Content-Length"]);
}

std::unique_ptr<std::string> Response::get_response_string() {
    return std::move(this->craft_response());
}