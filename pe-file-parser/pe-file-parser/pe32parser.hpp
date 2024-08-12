#pragma once

#include <memory>
#include <stdexcept>
#include <vector>
#include <winnt.h>

class PEParser
{
public:
	PEParser(std::unique_ptr<std::vector<char>> file_vector) : data_to_parse(std::move(file_vector)), m_dos_header(nullptr), m_nt_header(nullptr), section_headers(nullptr)
	{
        if (this->data_to_parse->empty()) {
            throw std::invalid_argument("File data is empty.");
        }

        if (this->data_to_parse->size() < sizeof(IMAGE_DOS_HEADER)) {
            throw std::runtime_error("File is too small to contain a valid DOS header.");
        }

        this->m_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(this->data_to_parse->data());

        if (this->m_dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            throw std::runtime_error("Invalid DOS signature.");
        }

        const std::uint32_t nt_header_offset = m_dos_header->e_lfanew;

        if (nt_header_offset >= this->data_to_parse->size()) {
            throw std::runtime_error("Invalid NT header offset.");
        }

		if (this->data_to_parse->size() < nt_header_offset + sizeof(IMAGE_NT_HEADERS)) {
            throw std::runtime_error("File is too small to contain a valid NT header.");
        }

        this->m_nt_header = reinterpret_cast<PIMAGE_NT_HEADERS32>(this->data_to_parse->data() + nt_header_offset);

        if (this->m_nt_header->Signature != IMAGE_NT_SIGNATURE) {
            throw std::runtime_error("Invalid NT signature.");
        }

		const uint32_t section_header_offset = this->m_dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS32);

        if (section_header_offset >= this->data_to_parse->size()) {
            throw std::runtime_error("Invalid section header offset.");
        }

        const uint32_t n_sections = this->m_nt_header->FileHeader.NumberOfSections;

        if (this->data_to_parse->size() < section_header_offset + n_sections * sizeof(IMAGE_SECTION_HEADER)) {
            throw std::runtime_error("File is too small to contain all section headers.");
        }

        const IMAGE_SECTION_HEADER* section_headers_t = reinterpret_cast<const IMAGE_SECTION_HEADER*>(this->data_to_parse->data() + section_header_offset);

        section_headers = std::make_shared<std::vector<IMAGE_SECTION_HEADER>>(n_sections);

		std::memcpy(this->section_headers->data(), section_headers_t, n_sections * sizeof(IMAGE_SECTION_HEADER));
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
};
