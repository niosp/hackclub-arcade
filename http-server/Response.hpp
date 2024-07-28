/*
 * Response class for the HTTP server: header file
 */

#ifndef HTTP_SERVER_RESPONSE_HPP
#define HTTP_SERVER_RESPONSE_HPP

#include <string>
#include <memory>
#include <unordered_map>

class Response {
private:
    std::shared_ptr<std::string> raw_body;
    std::unordered_map<std::string, std::string> headers;
    int status_code;
    std::string status_message;
    // final method -> builds the response after headers and body are set etc.
    std::unique_ptr<std::string> craft_response();
public:
    // constructor & destructor
    Response();
    ~Response();
    // raw body methods
    std::shared_ptr<std::string> get_raw_body();
    void set_raw_body(std::shared_ptr<std::string> raw_body);
    // headers methods
    void set_header(const std::string& key, const std::string& value);
    const std::string& get_header(const std::string& key);
    // status code methods
    void set_status_code(int status_code);
    int get_status_code() const;
    // status message methods
    void set_status_message(std::string status_message);
    std::string get_status_message();
    // content length methods
    void set_content_length(int content_length);
    int get_content_length();
    // response string methods
    std::unique_ptr<std::string> get_response_string();
};

#endif //HTTP_SERVER_RESPONSE_HPP
