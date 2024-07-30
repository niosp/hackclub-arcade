#include <windows.h>
#include <winnt.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <vector>

int locate(DWORD VA, PIMAGE_SECTION_HEADER section_headers, DWORD number_of_sections) {

    int index;

    for (int i = 0; i < number_of_sections; i++) {
        if (VA >= section_headers[i].VirtualAddress
            && VA < (section_headers[i].VirtualAddress + section_headers[i].Misc.VirtualSize)) {
            index = i;
            break;
        }
    }
    return index;
}

DWORD resolve(DWORD VA, int index, PIMAGE_SECTION_HEADER section_headers) {

    return (VA - section_headers[index].VirtualAddress) + section_headers[index].PointerToRawData;

}


DWORD rva_to_offset(DWORD rva, PIMAGE_SECTION_HEADER section_headers, int number_of_sections) {
    for (int i = 0; i < number_of_sections; i++) {
        PIMAGE_SECTION_HEADER section = &section_headers[i];
        DWORD section_start = section->VirtualAddress;
        DWORD section_end = section_start + section->Misc.VirtualSize;
        if (rva >= section_start && rva < section_end) {
            return (rva - section_start) + section->PointerToRawData;
        }
    }
    return 0;
}

int locate_virtual_address(DWORD VA, DWORD no_of_sections, IMAGE_SECTION_HEADER *header) {

    int index = 0;

    for (int i = 0; i < no_of_sections; i++) {
        if (VA >= header[i].VirtualAddress
            && VA < (header[i].VirtualAddress + header[i].Misc.VirtualSize)) {
            index = i;
            break;
        }
    }
    return index;
}

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
    std::printf("h0x%x\t\tAddress of NT header\n", parsed_dos_header->e_lfanew);
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

    if (parsed_nt_header.Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "Not a valid PE file\n";
        return -1;
    }

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
    std::printf("h0x%x\tADDITIONAL: Image base\n", parsed_optional_header.ImageBase);
    std::printf("h0x%x\t\tADDITIONAL: Section alignment\n", parsed_optional_header.SectionAlignment);
    std::printf("h0x%x\t\tADDITIONAL: Major os version\n", parsed_optional_header.MajorOperatingSystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Minor os version\n", parsed_optional_header.MinorOperatingSystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Major image version\n", parsed_optional_header.MajorImageVersion);
    std::printf("h0x%x\t\tADDITIONAL: Minor image version\n", parsed_optional_header.MinorImageVersion);
    std::printf("h0x%x\t\tADDITIONAL: Major subsystem version\n", parsed_optional_header.MajorSubsystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Minor subsystem version\n", parsed_optional_header.MinorSubsystemVersion);
    std::printf("h0x%x\t\tADDITIONAL: Win32 version value\n", parsed_optional_header.Win32VersionValue);
    std::printf("h0x%x\tADDITIONAL: Size of image\n", parsed_optional_header.SizeOfImage);
    std::printf("h0x%x\t\tADDITIONAL: Size of headers\n", parsed_optional_header.SizeOfHeaders);
    std::printf("h0x%x\tADDITIONAL: Checksum\n", parsed_optional_header.CheckSum);
    std::printf("h0x%x\t\tADDITIONAL: Subsystem\n", parsed_optional_header.Subsystem);
    std::printf("h0x%x\t\tADDITIONAL: DLL characteristics\n", parsed_optional_header.DllCharacteristics);
    std::printf("h0x%x\tADDITIONAL: Size of stack reserve\n", parsed_optional_header.SizeOfStackReserve);
    std::printf("h0x%x\t\tADDITIONAL: Size of stack commit\n", parsed_optional_header.SizeOfStackCommit);
    std::printf("h0x%x\tADDITIONAL: Size of heap reserve\n", parsed_optional_header.SizeOfHeapReserve);
    std::printf("h0x%x\t\tADDITIONAL: Size of heap commit\n", parsed_optional_header.SizeOfHeapCommit);
    std::printf("h0x%x\t\tADDITIONAL: Loader flags\n", parsed_optional_header.LoaderFlags);
    std::printf("h0x%x\t\tADDITIONAL: Number of RVA and sizes\n", parsed_optional_header.NumberOfRvaAndSizes);

    /*
     * sections:
     * -> .text: contains executable code
     * -> .data: contains initialized data
     * -> .rdata: contains read-only data
     * -> .bss: uninitialized data
     * -> .idata: import tables
     * -> .edata: export tables
     * -> .reloc: contains relocation information
     * -> .rsrc: contains resource information, like images or other embedded resources
     * -> .tls: information about running threads of the program (thread local storage)
     */

    for (int i = 0; i < parsed_nt_header.FileHeader.NumberOfSections; i++)
    {
        IMAGE_SECTION_HEADER section_header = { 0, 0, 0 };
        /*
         * address calculation:
         * buffer.data() -> the pointer to the start of the buffer
         * parsed_dos_header->e_lfanew -> buffer offset where NT header starts
         * sizeof(IMAGE_NT_HEADERS32) -> size of the NT header
         * (i * sizeof(IMAGE_SECTION_HEADER)) -> offset for section header i
         */
        section_header = *reinterpret_cast<PIMAGE_SECTION_HEADER>(buffer.data() + parsed_dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS32) + (i * sizeof(IMAGE_SECTION_HEADER)));

        std::printf("[SECTION %d]\n", i);
        std::printf("%c%c%c%c%c%c%c\t\tName\n", (char)section_header.Name[0],
            (char)section_header.Name[1],
            (char)section_header.Name[2],
            (char)section_header.Name[3],
            (char)section_header.Name[4],
            (char)section_header.Name[5],
            (char)section_header.Name[6],
            (char)section_header.Name[7]);
        std::printf("h0x%x\t\tVirtual size\n", section_header.Misc.VirtualSize);
        std::printf("h0x%x\t\tVirtual address\n", section_header.VirtualAddress);
        std::printf("h0x%x\t\tSize of raw data\n", section_header.SizeOfRawData);
        std::printf("h0x%x\t\tPointer to raw data\n", section_header.PointerToRawData);
        std::printf("h0x%x\t\tPointer to relocations\n", section_header.PointerToRelocations);
        std::printf("h0x%x\t\tPointer to line numbers\n", section_header.PointerToLinenumbers);
        std::printf("h0x%x\t\tNumber of relocations\n", section_header.NumberOfRelocations);
        std::printf("h0x%x\t\tNumber of line numbers\n", section_header.NumberOfLinenumbers);
        std::printf("h0x%x\t\tCharacteristics\n", section_header.Characteristics);
    }

    /*
        data directories (following section is from winnt.h!) ->
		// Directory Entries
		#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
		#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
		#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
		#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
		#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
		#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
		#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
		//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
		#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
		#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
		#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
		#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
		#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
		#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
		#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
		#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptorIMAGE_DATA_DIRECTORY

		Development notes:
        - data directories where *Address and *Size are zero, arent used!
    */

    DWORD number_of_sections = parsed_nt_header.FileHeader.NumberOfSections;
    IMAGE_DATA_DIRECTORY import_directory = parsed_optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]; // <- import directory, like explained above

    PIMAGE_SECTION_HEADER section_headers = reinterpret_cast<PIMAGE_SECTION_HEADER>(buffer.data() + parsed_dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS32));

    DWORD virtual_addr_import = import_directory.VirtualAddress;
    DWORD size_import = import_directory.VirtualAddress;

    DWORD fin_addr = rva_to_offset(virtual_addr_import, section_headers, number_of_sections);

    DWORD addr2 = resolve(import_directory.VirtualAddress,locate(import_directory.VirtualAddress, section_headers, number_of_sections), section_headers);

    int _import_directory_count = 0;
    bool found = false;
    /* loop through each DLL */
    while (!found) {
        IMAGE_IMPORT_DESCRIPTOR tmp;
        int offset = (_import_directory_count * sizeof(IMAGE_IMPORT_DESCRIPTOR)) + addr2;

        file_stream.seekg(offset, std::ios_base::beg);
        // fread(&tmp, sizeof(IMAGE_IMPORT_DESCRIPTOR), 1, file_stream);
        file_stream.read(reinterpret_cast<char*>(&tmp), sizeof(IMAGE_IMPORT_DESCRIPTOR));

        if(tmp.Name == 0)
        {
            break;
        }

        /* lookup the DLL name (convert the RVA address first) */
    	DWORD name_addr = rva_to_offset(tmp.Name, section_headers, number_of_sections);

        int name_size = 0;

        /* calculate size of the name */
        while (true) {
            char tmp_char;
            file_stream.seekg((name_addr + name_size), std::ios_base::beg);
            file_stream.read(&tmp_char, sizeof(char));
            if (tmp_char == 0x00) {
                break;
            }
            name_size++;
        }

        char* name = new char[name_size + 2];

        file_stream.seekg(name_addr, std::ios_base::beg);
        file_stream.read(name, (name_size * sizeof(char)) + 1);

        printf("%s:\n", name);
        delete[] name;

        /* -> name parsing & stuff finished */

        /* lookup the ILT */
        DWORD first_thunk = tmp.OriginalFirstThunk != 0 ? tmp.OriginalFirstThunk : tmp.FirstThunk;

        if(tmp.TimeDateStamp == 0)
        {
            std::cout << "BOUND: FALSE\n";
        }else if(tmp.TimeDateStamp == -1)
        {
            std::cout << "BOUND: TRUE\n";
        }

        DWORD arr_thunk_data = rva_to_offset(tmp.OriginalFirstThunk, section_headers, number_of_sections);

        DWORD counter_entries = 0;


        while(true)
        {
            IMAGE_THUNK_DATA32 thunk_data;
            DWORD thunk_data_addr = arr_thunk_data + (counter_entries * sizeof(IMAGE_THUNK_DATA32));
            file_stream.seekg(thunk_data_addr, std::ios_base::beg);
            file_stream.read(reinterpret_cast<char*>(&thunk_data), sizeof(IMAGE_THUNK_DATA32));

            IMAGE_IMPORT_BY_NAME import_entry;
            DWORD import_entry_addr = rva_to_offset(thunk_data.u1.AddressOfData, section_headers, number_of_sections);
            file_stream.seekg(import_entry_addr, std::ios_base::beg);
            file_stream.read(reinterpret_cast<char*>(&import_entry), sizeof(IMAGE_IMPORT_BY_NAME));



            counter_entries++;

        }

        _import_directory_count++;
    }


    return 0;
}
