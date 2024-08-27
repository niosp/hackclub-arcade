# PE32 File Parser
Small header-only library to parse PE32 files.

## Example
Just include the pe32parser.hpp file in your project. Only works on Windows systems due to the usage of WinAPI. If you prefer to use it without winnt.h, include the required structs from winnt.h. The constructor automatically calls the parsing function. 

Usage with filename:
```cpp
#include "pe32parser.hpp"

/* create the parser object */
PEParser pe_parser_instance("procmon.exe");

/* access the parsed data */
auto dos_header = pe_parser_instance.m_dos_header;
auto nt_header = pe_parser_instance.m_nt_header;
```

Usage with filestream pointer:
```cpp
#include <memory>
#include "pe32parser.hpp"

std::shared_ptr<std::ifstream> file_stream_ptr = std::make_shared<std::ifstream>("procmon.exe", std::ios::binary | std::ios::ate);
/* create the parser object */
PEParser pe_parser_instance(file_stream_ptr);

/* access the parsed data */
auto dos_header = pe_parser_instance.m_dos_header;
auto nt_header = pe_parser_instance.m_nt_header;
```

Usage with pre-filled vector:
```cpp
#include <memory>
#include "pe32parser.hpp"

/* create file stream */
std::ifstream file_stream(argv[1], std::ios::binary | std::ios::ate);
if (!file_stream.is_open()) {
    std::cerr << "Failed to open the file: " << argv[1] << "\n";
    return 1;
}

/* calculate the filesize */
std::streamsize file_size = file_stream.tellg();
file_stream.seekg(0, std::ios::beg);

std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>(file_size);

if (!file_stream.read(buffer->data(), file_size)) {
    std::cerr << "Failed to read the file into the buffer." << "\n";
    return 1;
}

/* create shared pointer to PEParser object, pass buffer pointer to constructor */
std::shared_ptr<PEParser> parser = std::make_shared<PEParser>(buffer);
```