#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
#include <winnt.h>
#include <iostream>
#include <string>
#include <bitset>

class ImageResourceDirW;
class ImageResourceDirEntryW;

using DirEntryW_Vec = std::shared_ptr<std::vector<std::shared_ptr<ImageResourceDirEntryW>>>;

class ImageResourceDirEntryW
{
public:
    ImageResourceDirEntryW(PIMAGE_RESOURCE_DIRECTORY_ENTRY p_dir_entry) : dir_entry_ptr(p_dir_entry)
    {
        this->nested_directories = std::make_shared<std::vector<std::shared_ptr<ImageResourceDirW>>>();
    }

    void insert_nested_directory(std::shared_ptr<ImageResourceDirW> p_dir) const
    {
        this->nested_directories->emplace_back(p_dir);
    }

    DWORD get_data_is_directory() const
    {
        return (this->dir_entry_ptr->OffsetToData >> 31) & 1;
    }

    DWORD get_name_is_string() const
    {
        return (this->dir_entry_ptr->OffsetToData >> 31) & 1;
    }

    DWORD get_offset_to_directory() const
    {
        return (this->dir_entry_ptr->OffsetToData & 0x7FFFFFFF);
    }

    PIMAGE_RESOURCE_DATA_ENTRY get_stored_resource_data() const
    {
        return this->resource_data_entry_p;
    }

    void insert_resource_data(PIMAGE_RESOURCE_DATA_ENTRY p_resource_data)
    {
        this->resource_data_entry_p = p_resource_data;
    }

private:
    PIMAGE_RESOURCE_DIRECTORY_ENTRY dir_entry_ptr;
    PIMAGE_RESOURCE_DATA_ENTRY resource_data_entry_p;
    std::shared_ptr<std::vector<std::shared_ptr<ImageResourceDirW>>> nested_directories;
};

class ImageResourceDirW
{
public:
    ImageResourceDirW(PIMAGE_RESOURCE_DIRECTORY p_dir) : dir_ptr(p_dir)
    {
        this->resource_entries = std::make_shared<std::vector<std::shared_ptr<ImageResourceDirEntryW>>>();
    }

    void insert_directory_entry(std::shared_ptr<ImageResourceDirEntryW> p_entry) const
    {
        this->resource_entries->emplace_back(p_entry);
    }

    PIMAGE_RESOURCE_DIRECTORY get_resource_directory() const
    {
        return this->dir_ptr;
    }

    DirEntryW_Vec get_resource_entries() const
    {
        return this->resource_entries;
    }
private:
    PIMAGE_RESOURCE_DIRECTORY dir_ptr;
    DirEntryW_Vec resource_entries;
};

class PEDLLType
{
public:
    PEDLLType(std::string p_name, std::vector<std::string> p_dll_names, IMAGE_IMPORT_DESCRIPTOR p_descr) : name(p_name), dll_names(p_dll_names), descr(p_descr) {};
private:
    std::string name;
    std::vector<std::string> dll_names;
    IMAGE_IMPORT_DESCRIPTOR descr;
};

class PEParser
{
public:
    PEParser(std::shared_ptr<std::vector<char>> file_vector) : data_to_parse(std::move(file_vector)), m_dos_header(nullptr), m_nt_header(nullptr), section_headers(nullptr)
    {
        /* check passed data (vector) */
        if (this->data_to_parse->empty()) {
            throw std::invalid_argument("File data is empty.");
        }

        /* prepare parsing the DOS header */
        if (this->data_to_parse->size() < sizeof(IMAGE_DOS_HEADER)) {
            throw std::runtime_error("File is too small to contain a valid DOS header.");
        }

        /* parse the DOS header */
        this->m_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(this->data_to_parse->data());

        /* validate DOS signature */
        if (this->m_dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            throw std::runtime_error("Invalid DOS signature.");
        }

        /* calculate offset of the NT headers */
        const std::uint32_t nt_header_offset = m_dos_header->e_lfanew;

        /* calculated offset invalid? */
        if (nt_header_offset >= this->data_to_parse->size()) {
            throw std::runtime_error("Invalid NT header offset.");
        }

        /* file size check */
        if (this->data_to_parse->size() < nt_header_offset + sizeof(IMAGE_NT_HEADERS)) {
            throw std::runtime_error("File is too small to contain a valid NT header.");
        }

        /* parse the NT headers (32bit) */
        this->m_nt_header = reinterpret_cast<PIMAGE_NT_HEADERS32>(this->data_to_parse->data() + nt_header_offset);

        /* validate the NT signature */
        if (this->m_nt_header->Signature != IMAGE_NT_SIGNATURE) {
            throw std::runtime_error("Invalid NT signature.");
        }

        /* calculate offset of section headers (DOS + size of NT headers) */
        const uint32_t section_header_offset = this->m_dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS32);

        /* calculated offset invalid? */
        if (section_header_offset >= this->data_to_parse->size()) {
            throw std::runtime_error("Invalid section header offset.");
        }

        /* retrieve number of sections present in loaded file */
        const DWORD n_sections = this->m_nt_header->FileHeader.NumberOfSections;

        /* verify filesize */
        if (this->data_to_parse->size() < section_header_offset + n_sections * sizeof(IMAGE_SECTION_HEADER)) {
            throw std::runtime_error("File is too small to contain all section headers.");
        }

        /* get pointer to first section header object */
        const IMAGE_SECTION_HEADER* section_headers_t = reinterpret_cast<const IMAGE_SECTION_HEADER*>(this->data_to_parse->data() + section_header_offset);

        /* create shared ptr to vec storing section headers later on */
        section_headers = std::make_shared<std::vector<IMAGE_SECTION_HEADER>>(n_sections);

        /* copy section header information from pointer above (size = n_sections * sizeof(IMAGE_SECTION_HEADER)) */
        std::memcpy(this->section_headers->data(), section_headers_t, n_sections * sizeof(IMAGE_SECTION_HEADER));

        /* get import directory */
        IMAGE_DATA_DIRECTORY import_directory = this->get_optional_headers()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

        /* get pointer to first section header */
        PIMAGE_SECTION_HEADER section_headers = reinterpret_cast<PIMAGE_SECTION_HEADER>(this->data_to_parse->data() + this->m_dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS32));

        /* VirtualAddress of the imports */
        DWORD virtual_addr_import = import_directory.VirtualAddress;

        /* file offset */
        DWORD fin_addr = rva_to_offset(virtual_addr_import, section_headers, n_sections);

        int import_directory_count = 0;

        /* std::vector with DLL entries */
        imports = std::make_shared<std::vector<PEDLLType>>();

        /* while no empty descriptor was found */
        while (true) {
            IMAGE_IMPORT_DESCRIPTOR tmp;

            /* calculate file offset (offset from above + n * descriptor) */
            int offset = (import_directory_count * sizeof(IMAGE_IMPORT_DESCRIPTOR)) + fin_addr;

            std::memcpy(&tmp, this->data_to_parse->data() + offset, sizeof(IMAGE_IMPORT_DESCRIPTOR));

            /* descriptor with empty fields -> break */
            if (tmp.Name == 0)
            {
                break;
            }

            /* process DLL, get the name */
            DWORD name_addr = rva_to_offset(tmp.Name, section_headers, n_sections);

            int name_size = 0;

            /* calculate size of the name */
            while (true) {
                char tmp_char;
                std::memcpy(&tmp_char, this->data_to_parse->data() + (name_addr + name_size), sizeof(char));
                if (tmp_char == 0x00) {
                    break;
                }
                name_size++;
            }

            char* fin_name = new char[name_size + 2];

            std::memcpy(fin_name, this->data_to_parse->data() + name_addr, (name_size * sizeof(char)) + 1);

            /* -> name parsing & stuff finished */

            /* lookup the ILT */
            DWORD first_thunk = tmp.OriginalFirstThunk != 0 ? tmp.OriginalFirstThunk : tmp.FirstThunk;

            if (tmp.TimeDateStamp == 0)
            {
                std::cout << "BOUND: FALSE\n";
            }
            else if (tmp.TimeDateStamp == -1)
            {
                std::cout << "BOUND: TRUE\n";
            }

            /* file offset to thunk data */
            DWORD arr_thunk_data = rva_to_offset(tmp.OriginalFirstThunk, section_headers, n_sections);

            std::vector<std::string> function_names = {};

            IMAGE_THUNK_DATA32* thunk_data = reinterpret_cast<IMAGE_THUNK_DATA32*>(this->data_to_parse->data() + arr_thunk_data);

            while (thunk_data->u1.AddressOfData) {
                if (thunk_data->u1.AddressOfData & IMAGE_ORDINAL_FLAG32) {
                    /* import by ordinal (without function name) */
                    DWORD ordinal = thunk_data->u1.Ordinal & 0xFFFF;
                    function_names.push_back("Ordinal_" + std::to_string(ordinal));
                }
                else {
                    /* normal import by name */
                    IMAGE_IMPORT_BY_NAME* import_by_name = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(this->data_to_parse->data() + rva_to_offset(thunk_data->u1.AddressOfData, section_headers, n_sections));
                    function_names.emplace_back(import_by_name->Name);
                }
                thunk_data++;
            }

            std::string dll_name(fin_name);

            PEDLLType dll_temp(dll_name, function_names, tmp);

            imports->emplace_back(dll_temp);

            delete[] fin_name;

            import_directory_count++;
        }

        /* set to enable later usage of rva calculator (non-static) */
        this->n_sections_g = n_sections;
        this->section_hdrs_p = section_headers;

        /* get base relocation directory */
        IMAGE_DATA_DIRECTORY directory_entry_basereloc_rva = this->get_optional_headers()->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

        /* calculate file offset of basereloc directory */
        DWORD base_relocation_directory_addr = rva_to_offset(directory_entry_basereloc_rva.VirtualAddress);

        DWORD basereloc_size_counter = 0;
        DWORD basereloc_directory_counter = 0;

        this->base_relocs = std::make_shared<std::vector<IMAGE_BASE_RELOCATION>>();

        while (true)
        {
            IMAGE_BASE_RELOCATION temp_base_relo;

            /* add base reloc blocks on top to the directory file offset */
            DWORD base_relocation_offset = (basereloc_size_counter + base_relocation_directory_addr);

            std::memcpy(&temp_base_relo, this->data_to_parse->data() + base_relocation_offset, sizeof(IMAGE_BASE_RELOCATION));

            /* last reloc reached, terminate */
            if (temp_base_relo.SizeOfBlock == 0 && temp_base_relo.VirtualAddress == 0)
            {
                break;
            }

            this->base_relocs->emplace_back(temp_base_relo);

            basereloc_directory_counter++;
            basereloc_size_counter += temp_base_relo.SizeOfBlock;
        }

        /* base reloc parsing done */

        /* .rsrc */
        /* get the data directory itself */
        IMAGE_DATA_DIRECTORY resource_directory = this->get_optional_headers()->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE];

        DWORD resource_size = resource_directory.Size;
        DWORD resource_rva = resource_directory.VirtualAddress;

        /* convert the rva from resource_directory->VirtualAddress to the file offset (so resource section starts at resource_offset */
        DWORD resource_offset = rva_to_offset(resource_rva);

        /* "root" resource directory */
        PIMAGE_RESOURCE_DIRECTORY dir = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(this->data_to_parse->data() + resource_offset);

        bool end_in_dir_reached = false;

        DWORD ctr = 0;

        this->root_dir = std::make_shared<ImageResourceDirW>(dir);

        if (resource_directory.Size != 0)
        {
            /* entries inside root (/) resource directory */
            for (int i = 0; i < dir->NumberOfIdEntries + dir->NumberOfNamedEntries; i++)
            {
                /* get the entry */

                PIMAGE_RESOURCE_DIRECTORY_ENTRY dir_entry_1_p = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(this->data_to_parse->data() + resource_offset + sizeof(IMAGE_RESOURCE_DIRECTORY) + i * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

                std::shared_ptr<ImageResourceDirEntryW> dir_entry_1_s_p = std::make_shared<ImageResourceDirEntryW>(dir_entry_1_p);

                root_dir->insert_directory_entry(dir_entry_1_s_p);

                /* get the directory the entry points to */
                PIMAGE_RESOURCE_DIRECTORY dir_1_p;
                DWORD dir_1_offset = dir_entry_1_p->OffsetToDirectory + resource_offset;

                dir_1_p = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(this->data_to_parse->data() + dir_1_offset);

                std::shared_ptr<ImageResourceDirW> dir_1_s_p = std::make_shared<ImageResourceDirW>(dir_1_p);

            	dir_entry_1_s_p->insert_nested_directory(dir_1_s_p);

                /* iterate through the directory mentioned in last comment */
                for (int j = 0; j < dir_1_p->NumberOfNamedEntries + dir_1_p->NumberOfIdEntries; j++)
                {
                    /* get the directory entry */
                    PIMAGE_RESOURCE_DIRECTORY_ENTRY dir_entry_2_p = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(this->data_to_parse->data() + dir_1_offset + sizeof(IMAGE_RESOURCE_DIRECTORY) + j * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

                    std::shared_ptr<ImageResourceDirEntryW> dir_entry_2_s_p = std::make_shared<ImageResourceDirEntryW>(dir_entry_2_p);

                    dir_1_s_p->insert_directory_entry(dir_entry_2_s_p);

                    /* get nested resource directory */
                    DWORD dir_2_offset = dir_entry_2_p->OffsetToDirectory + resource_offset;

                    PIMAGE_RESOURCE_DIRECTORY dir_2_p = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(this->data_to_parse->data() + dir_2_offset);

                    std::shared_ptr<ImageResourceDirW> dir_2_s_p = std::make_shared<ImageResourceDirW>(dir_2_p);

                    dir_entry_2_s_p->insert_nested_directory(dir_2_s_p);

                    /* for every entry in directory dir_2 */
                    for (int k = 0; k < dir_2_p->NumberOfNamedEntries + dir_2_p->NumberOfIdEntries; k++)
                    {
                        /* receive last & final entry (resource directory entry) */
                        PIMAGE_RESOURCE_DIRECTORY_ENTRY dir_entry_3_p = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(this->data_to_parse->data() + dir_2_offset + sizeof(IMAGE_RESOURCE_DIRECTORY) + k * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY));

                        std::shared_ptr<ImageResourceDirEntryW> dir_entry_3_ptr = std::make_shared<ImageResourceDirEntryW>(dir_entry_3_p);

                        dir_2_s_p->insert_directory_entry(dir_entry_3_ptr);

                        PIMAGE_RESOURCE_DATA_ENTRY data_entry = reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(this->data_to_parse->data() + dir_entry_3_p->OffsetToData + resource_offset);

                        dir_entry_3_ptr->insert_resource_data(data_entry);
                    }

                }
            }
        }

        /* debug parsing */

        IMAGE_DATA_DIRECTORY debug_directory = this->get_optional_headers()->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];

        DWORD debug_directory_rva = rva_to_offset(debug_directory.VirtualAddress);

        this->debug_dir_p = reinterpret_cast<PIMAGE_DEBUG_DIRECTORY>(this->data_to_parse->data() + debug_directory_rva);

        /* tls (thread local storage) parsing */

        IMAGE_DATA_DIRECTORY tls_directory = this->get_optional_headers()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];

        DWORD tls_directory_rva = rva_to_offset(debug_directory.VirtualAddress);

        this->tls_dir_p = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(this->data_to_parse->data() + tls_directory_rva);
    }

    PIMAGE_DOS_HEADER get_dos_header() const {
        return this->m_dos_header;
    }

    PIMAGE_NT_HEADERS32 get_nt_headers() const {
        return this->m_nt_header;
    }

    std::unique_ptr<IMAGE_OPTIONAL_HEADER32> get_optional_headers() const
    {
        return std::make_unique<IMAGE_OPTIONAL_HEADER32>(this->m_nt_header->OptionalHeader);
    }

    static DWORD rva_to_offset(DWORD rva, PIMAGE_SECTION_HEADER p_section_headers, int p_number_of_sections) {
        for (int i = 0; i < p_number_of_sections; i++) {
            const PIMAGE_SECTION_HEADER section = &p_section_headers[i];
            const DWORD section_start = section->VirtualAddress;
            const DWORD section_end = section_start + section->Misc.VirtualSize;

            if (rva >= section_start && rva < section_end) {
                return (rva - section_start) + section->PointerToRawData;
            }
        }
        return 0;
    }

    DWORD rva_to_offset(DWORD rva) const {
        for (int i = 0; i < this->n_sections_g; i++) {
            const PIMAGE_SECTION_HEADER section = &this->section_hdrs_p[i];
            const DWORD section_start = section->VirtualAddress;
            const DWORD section_end = section_start + section->Misc.VirtualSize;

            if (rva >= section_start && rva < section_end) {
                return (rva - section_start) + section->PointerToRawData;
            }
        }
        return 0;
    }

private:
    std::shared_ptr<std::vector<char>> data_to_parse;
    std::shared_ptr<std::vector<IMAGE_SECTION_HEADER>> section_headers;
    std::shared_ptr<std::vector<PEDLLType>> imports;
    std::shared_ptr<std::vector<IMAGE_BASE_RELOCATION>> base_relocs;
    std::shared_ptr<ImageResourceDirW> root_dir;
    PIMAGE_DOS_HEADER m_dos_header;
    PIMAGE_NT_HEADERS32 m_nt_header;
    PIMAGE_OPTIONAL_HEADER32 m_opt_nt_header;
    PIMAGE_SECTION_HEADER section_hdrs_p;
    PIMAGE_DEBUG_DIRECTORY debug_dir_p;
    PIMAGE_TLS_DIRECTORY tls_dir_p;
    DWORD n_sections_g;
};
