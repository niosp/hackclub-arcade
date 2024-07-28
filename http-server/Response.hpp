//
// Created by User on 28/07/2024.
//

#ifndef HTTP_SERVER_RESPONSE_HPP
#define HTTP_SERVER_RESPONSE_HPP

#include <string>
#include <memory>

class Response {
private:
    std::shared_ptr<std::string> raw_body;
public:
    // constructor & destructor
    Response();
    ~Response();
    // final method -> builds the response after headers and body are set etc.
    void craft_response();
    // raw body methods
    std::shared_ptr<std::string> get_raw_body();
    void set_raw_body(std::shared_ptr<std::string> raw_body);
    // headers methods
    void set_header(std::string key, std::string value);
    std::string get_header(std::string key);
    // status code methods
    void set_status_code(int status_code);
    int get_status_code();
    // status message methods
    void set_status_message(std::string status_message);
    std::string get_status_message();
    // content type methods
    void set_content_type(std::string content_type);
    std::string get_content_type();
    // content length methods
    void set_content_length(int content_length);
    int get_content_length();
    // response string methods
    std::unique_ptr<std::string> get_response_string();
};


#endif //HTTP_SERVER_RESPONSE_HPP
