//
// Created by User on 29/07/2024.
//

#include <cstdint>
#include <filesystem>
#include "ResponseTypes.hpp"
#include "util.hpp"

std::shared_ptr<std::string> ResponseTypes::generate_directory_listing(const std::string &path) {
    std::shared_ptr<std::string> body = std::make_shared<std::string>();
    *body += "<!DOCTYPE html>\n";
    *body += "<html>\n";
    *body += "<head>\n";
    *body += "    <title>Directory Listing</title>\n";
    *body += "    <style>\n";
    *body += "        body { font-family: Arial, sans-serif; }\n";
    *body += "        table { width: 100%; border-collapse: collapse; }\n";
    *body += "        th, td { border: 1px solid #ddd; padding: 8px; }\n";
    *body += "        th { background-color: #f2f2f2; }\n";
    *body += "    </style>\n";
    *body += "</head>\n";
    *body += "<body>\n";
    *body += "    <h1>Directory Listing for ";
    *body += "." + path;
    *body += "</h1>\n";
    *body += "    <table>\n";
    *body += "        <tr>\n";
    *body += "            <th>Name</th>\n";
    *body += "            <th>Type</th>\n";
    *body += "            <th>Size (bytes)</th>\n";
    *body += "        </tr>\n";
    *body += "        <tr>\n";
    *body += "            <td><a href=\"../\">Go back</a></td>\n";
    *body += "        </tr>\n";
    // iterate over the directory entries, add them to the table
    for (const auto& entry : std::filesystem::directory_iterator("." + path)) {
        std::string name = entry.path().filename().string();
        std::string type = std::filesystem::is_directory(entry.status()) ? "Directory" : "File";
        std::uintmax_t size = std::filesystem::is_regular_file(entry.status()) ? std::filesystem::file_size(entry.path()) : 0;
        *body += "        <tr>\n";
        *body += "              <td><a href=\"";
        if(std::filesystem::is_directory(entry.status())){
            *body += name + "/";
        }else{
            *body += name;
        }
        *body += "\">";
        *body += name;
        *body += "</a></td>\n";
        *body += "            <td>" + type + "</td>\n";
        *body += "            <td>";
        *body += std::to_string(size);
        *body += "</td>\n";
        *body += "        </tr>\n";
    }
    *body += "    </table>\n";
    *body += "</body>\n";
    *body += "</html>\n";
    return body;
}

std::shared_ptr<std::string> ResponseTypes::generate_404_not_found(const std::string &path) {
    std::unique_ptr<std::string> not_found_ptr = read_file_from_disk("404.html");
    if(not_found_ptr){
        return std::make_shared<std::string>(*not_found_ptr);
    }else{
        return std::make_shared<std::string>(
                "<title>HTTP-Server - File not found</title>"
                "<header>Can't find the specified 404 NOT FOUND file</header>"
        );
    }
}
