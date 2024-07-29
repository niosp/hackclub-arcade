//
// Created by User on 29/07/2024.
//
#include <chrono>
#include "util.hpp"
std::unique_ptr<std::string> read_file_from_disk(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Could not open file: " << filePath << "\n";
        return nullptr;
    }else{
        std::string contents;
        file.seekg(0, std::ios::end);
        contents.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&contents[0], contents.size());
        file.close();
        return(std::make_unique<std::string>(contents));
    }
}

std::string get_current_timestamp() {
    // get current time, result will look like this: Mon Jul 29 16:42:04 2024
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    return std::ctime(&now_c);
}

bool is_html_file(const std::string& filename){
    try{
        return filename.substr(filename.length() - 5) == ".html";
    }catch(std::out_of_range& e){
        return false;
    }
}