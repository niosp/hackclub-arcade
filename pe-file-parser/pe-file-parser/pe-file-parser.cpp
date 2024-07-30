#include <windows.h>
#include <winnt.h>
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
    std::printf("h0x%x\t\tMagic number\n", parsed_dos_header->e_magic);
    std::printf("h0x%x\t\tPages in file\n", parsed_dos_header->e_cp);
    std::printf("h0x%x\t\tBytes last page\n", parsed_dos_header->e_cblp);
    std::printf("h0x%x\t\tSize of header in paragraphs\n", parsed_dos_header->e_cparhdr);
    std::printf("h0x%x\t\tRelocations\n", parsed_dos_header->e_crlc);
    std::printf("h0x%x\t\tMin allocation\n", parsed_dos_header->e_minalloc);
    std::printf("h0x%x\t\tMax allocation\n", parsed_dos_header->e_maxalloc);
    std::printf("h0x%x\t\tInitial SS value\n", parsed_dos_header->e_ss);
    std::printf("h0x%x\t\tInitial SP value\n", parsed_dos_header->e_sp);
    std::printf("h0x%x\t\tChecksum\n", parsed_dos_header->e_csum);
    std::printf("h0x%x\t\tInitial IP value\n", parsed_dos_header->e_ip);
    std::printf("h0x%x\t\tInitial CS value\n", parsed_dos_header->e_cs);
    std::printf("h0x%x\t\tOverlay number\n", parsed_dos_header->e_ovno);
    std::printf("h0x%x\t\tAddress of new exe header\n", parsed_dos_header->e_lfanew);
    std::printf("h0x%x\t\tOEM information\n", parsed_dos_header->e_oeminfo);
    std::printf("h0x%x\t\tOEM id\n", parsed_dos_header->e_oemid);
    std::printf("h0x%x\t\tAddress of the relocation table\n", parsed_dos_header->e_lfarlc);

    // file_stream.seekg(parsed_dos_header->e_lfanew, std::ios::beg);

    /*
     * parsing the IMAGE_NT_HEADERS32 structure
     * (32bit because 64 would be more work to support, maybe later)
     *
     * Signature: a DWORD (value 0x50450000) -> identifies the file in PE format
     * IMAGE_FILE_HEADER:
     *  -> Machine: a WORD (e.g. 0x14c for x86)
     *  -> NumberOfSections: word -> number of sections
     *  -> TimeDateStamp: DWORD -> unix timestamp of the file creation
     *  -> PointerToSymbolTable: DWORD -> deprecated
     *  -> sizeOfOptionalHeader: WORD -> size of the optional header
     *  -> Characteristics: WORD -> stores file attributes like execution bits, file type (https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#characteristics)
     *      important: WORD in winapi is 16bit, so 16 flags can be stored
     */

    IMAGE_NT_HEADERS32 parsed_nt_header = {0, 0, 0};
    parsed_nt_header = *reinterpret_cast<PIMAGE_NT_HEADERS32>(buffer.data() + parsed_dos_header->e_lfanew);

    std::printf("[NT HEADER]\n");
    std::printf("h0x%x\t\tSignature\n", parsed_nt_header.Signature);
    std::printf("[NT HEADER - IMAGE_NT_HEADERS32\n");
    std::printf("h0x%x\t\tFILE header: machine\n", parsed_nt_header.FileHeader.Machine);
    std::printf("h0x%x\t\tFILE header: number of sections\n", parsed_nt_header.FileHeader.NumberOfSections);
    std::printf("h0x%x\tFILE header: unix timestamp (seconds)\n", parsed_nt_header.FileHeader.TimeDateStamp);
    std::printf("h0x%x\t\tFILE header: pointer to symbol table\n", parsed_nt_header.FileHeader.PointerToSymbolTable);
    std::printf("h0x%x\t\tFILE header: size of optional header\n", parsed_nt_header.FileHeader.SizeOfOptionalHeader);
    std::printf("h0x%x\t\tFILE header: characteristics (flags)\n", parsed_nt_header.FileHeader.Characteristics);

    for(int i=0; i < 16; i++)
    {
        bool value = (parsed_nt_header.FileHeader.Characteristics & (1 << i)) >> i;
        if(value)
        {
            /*
             * the flags are defined in the winnt.h header file, or you take a look at the link from comment above
             */
            std::printf("\t\t\t-> characteristics: bit %d is set\n", i+1);
        }
	    
    }

    /*
     * parsing the IMAGE_OPTIONAL_HEADER32 structure
     */

    IMAGE_OPTIONAL_HEADER32 parsed_optional_header = {0, 0, 0};
    parsed_optional_header = parsed_nt_header.OptionalHeader;

    /* just the default members */
    std::printf("[NT HEADERS - IMAGE_OPTIONAL_HEADER32]\n");
    std::printf("h0x%x\t\tDEFAULT: Magic\n", parsed_optional_header.Magic);
    std::printf("h0x%x\t\tDEFAULT: Major linker version\n", parsed_optional_header.MajorLinkerVersion);
    std::printf("h0x%x\t\tDEFAULT: Minor linker version\n", parsed_optional_header.MinorLinkerVersion);
    std::printf("h0x%x\tDEFAULT: Size of code\n", parsed_optional_header.SizeOfCode);
    std::printf("h0x%x\tDEFAULT: Size of initialized data\n", parsed_optional_header.SizeOfInitializedData);
    std::printf("h0x%x\t\tDEFAULT: Size of uninitialized data\n", parsed_optional_header.SizeOfUninitializedData);
    std::printf("h0x%x\tDEFAULT: Address of the entrypoint\n", parsed_optional_header.AddressOfEntryPoint);
    std::printf("h0x%x\t\tDEFAULT: Base of code\n", parsed_optional_header.BaseOfCode);
    std::printf("h0x%x\tDEFAULT: Base of data\n", parsed_optional_header.BaseOfData);

    /* optional values needed by the windows pe loader */
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.ImageBase);
    std::printf("h0x%x\t\tADDITIONAL: Section alignment\n", parsed_optional_header.SectionAlignment);
    std::printf("h0x%x\t\tADDITIONAL: Major os version\n", parsed_optional_header.MajorOperatingSystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Minor os version\n", parsed_optional_header.MinorOperatingSystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Major image version\n", parsed_optional_header.MajorImageVersion);
    std::printf("h0x%x\t\tADDITIONAL: Minor image version\n", parsed_optional_header.MinorImageVersion);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.MajorSubsystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.MinorSubsystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.Win32VersionValue);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.SizeOfImage);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.SizeOfHeaders);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.CheckSum);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.Subsystem);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.DllCharacteristics);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.SizeOfStackReserve);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.SizeOfStackCommit);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.SizeOfHeapReserve);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.SizeOfHeapCommit);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.LoaderFlags);
    std::printf("h0x%x\t\tADDITIONAL: Image base\n", parsed_optional_header.NumberOfRvaAndSizes);






    return 0;
}
