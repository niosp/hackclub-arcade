//
// Created by User on 29/07/2024.
//
#include "util.hpp"
std::unique_ptr<std::string> read_file_from_disk(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Could not open file: " << filePath << "\n";
        return nullptr;
    }else{
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        std::cout << "File: " << filePath << " read successfully\n";
        std::cout << "Size: " << buffer.str().size() << " bytes\n";
        return std::make_unique<std::string>(buffer.str());
    }
}