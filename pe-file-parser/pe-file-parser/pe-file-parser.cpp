#include <windows.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <vector>

int main(int argc, char* argv[])
{
    if(argc <= 1) {
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

    std::vector<uint8_t> buffer(file_size);

    if (!file_stream.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        std::cerr << "Failed to read the file into the buffer." << "\n";
        return 1;
    }

    std::printf("File read successfully. Size: %d bytes\n", file_size);

    // DOS header information from: https://offwhitesecurity.dev/malware-development/portable-executable-pe/dos-header/
    // sizeof(struct IMAGE_DOS_HEADER) = 64 bytes!
    PIMAGE_DOS_HEADER parsed_dos_header = nullptr;
    parsed_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(buffer.data());

    std::printf("[DOS HEADER]\n");
    std::printf("\t* h0x%x\t\tMagic number\n", parsed_dos_header->e_magic);
    std::printf("\t* h0x%x\t\tPages in file\n", parsed_dos_header->e_cp);
    std::printf("\t* h0x%x\t\tBytes last page\n", parsed_dos_header->e_cblp);
    std::printf("\t* h0x%x\t\tSize of header in paragraphs\n", parsed_dos_header->e_cparhdr);
    std::printf("\t* h0x%x\t\tRelocations\n", parsed_dos_header->e_crlc);
    std::printf("\t* h0x%x\t\t\tMin allocation\n", parsed_dos_header->e_minalloc);
    std::printf("\t* h0x%x\t\t\tMax allocation\n", parsed_dos_header->e_maxalloc);
    std::printf("\t* h0x%x\t\t\tInitial SS value\n", parsed_dos_header->e_ss);
    std::printf("\t* h0x%x\t\tInitial SP value\n", parsed_dos_header->e_sp);
    std::printf("\t* h0x%x\t\tChecksum\n", parsed_dos_header->e_csum);
    std::printf("\t* h0x%x\t\tInitial IP value\n", parsed_dos_header->e_ip);
    std::printf("\t* h0x%x\t\tInitial CS value\n", parsed_dos_header->e_cs);
    std::printf("\t* h0x%x\t\tOverlay number\n", parsed_dos_header->e_ovno);
    std::printf("\t* h0x%x\t\tAddress of new exe header\n", parsed_dos_header->e_lfanew);
    std::printf("\t* h0x%x\t\tOEM information\n", parsed_dos_header->e_oeminfo);
    std::printf("\t* h0x%x\t\tOEM id\n", parsed_dos_header->e_oemid);
    std::printf("\t* h0x%x\t\tAddress of the relocation table\n", parsed_dos_header->e_lfarlc);



    return 0;
}
