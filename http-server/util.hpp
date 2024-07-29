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

std::unique_ptr<std::string> read_file_from_disk(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Could not open file: " << filePath << "\n";
        return nullptr;
    }else{
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        return std::make_unique<std::string>(buffer.str());
    }
}

#endif //HTTP_SERVER_UTIL_HPP