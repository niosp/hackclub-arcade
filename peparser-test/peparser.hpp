#pragma once

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
#include <winnt.h>

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
        const uint32_t n_sections = this->m_nt_header->FileHeader.NumberOfSections;

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
            DWORD name_offset = rva_to_offset(virtual_addr_import, section_headers, n_sections);

            int name_size = 0;

            /* calculate size of the name */
            while (true) {
                char tmp_char;
                std::memcpy(&tmp_char, this->data_to_parse->data() + (name_offset + name_size), sizeof(char));
                if (tmp_char == 0x00) {
                    break;
                }
                name_size++;
            }

            char* fin_name = new char[name_size + 2];

            std::memcpy(&fin_name, this->data_to_parse->data() + name_offset, (name_size * sizeof(char)) + 1);

            printf("%s:\n", fin_name);
            delete[] fin_name;
        }

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

private:
    std::shared_ptr<std::vector<char>> data_to_parse;
    std::shared_ptr<std::vector<IMAGE_SECTION_HEADER>> section_headers;
    PIMAGE_DOS_HEADER m_dos_header;
    PIMAGE_NT_HEADERS32 m_nt_header;
    PIMAGE_OPTIONAL_HEADER32 m_opt_nt_header;
};
