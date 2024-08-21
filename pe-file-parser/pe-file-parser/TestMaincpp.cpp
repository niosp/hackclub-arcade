#include <windows.h>
#include <winnt.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <vector>

#include "pe32parser.hpp"

int changeme(int argc, char* argv[])
{
    if (argc <= 1) {
        std::cerr << "Usage: " << argv[0] << " <PE file path>\n";
        return 0;
    }

    std::ifstream file_stream(argv[1], std::ios::binary | std::ios::ate);
    if (!file_stream.is_open()) {
        std::cerr << "Failed to open the file: " << argv[1] << "\n";
        return 1;
    }

    std::streamsize file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>(file_size);

    if (!file_stream.read(buffer->data(), file_size)) {
        std::cerr << "Failed to read the file into the buffer." << "\n";
        return 1;
    }

    std::shared_ptr<PEParser> parser = std::make_shared<PEParser>(buffer);

    std::printf("File read successfully. Size: %td bytes\n", file_size);

    return 0;
}
