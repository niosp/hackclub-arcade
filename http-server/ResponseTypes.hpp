//
// Created by User on 29/07/2024.
//

#ifndef HTTP_SERVER_RESPONSETYPES_HPP
#define HTTP_SERVER_RESPONSETYPES_HPP

#include <string>
#include <memory>

class ResponseTypes {
    ResponseTypes();
    ~ResponseTypes();
    std::unique_ptr<std::string> generate_directory_listing(const std::string& path);
    std::unique_ptr<std::string> generate_404_not_found(const std::string& path);
};


#endif //HTTP_SERVER_RESPONSETYPES_HPP
