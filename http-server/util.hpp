//
// Created by User on 29/07/2024.
//

#ifndef HTTP_SERVER_UTIL_HPP
#define HTTP_SERVER_UTIL_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

std::unique_ptr<std::string> read_file_from_disk(const std::string& filePath);
std::string get_current_timestamp();
bool is_html_file(const std::string& filename);

#endif //HTTP_SERVER_UTIL_HPP