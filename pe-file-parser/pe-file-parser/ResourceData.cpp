#include "ResourceData.hpp"

ResourceData::ResourceData(DWORD p_offset_to_data, DWORD p_size, DWORD p_code_page)
{
	this->offset_to_data = p_offset_to_data;
	this->size = p_size;
	this->code_page = p_code_page;
}

DWORD ResourceData::get_code_page() const
{
	return this->code_page;
}

DWORD ResourceData::get_offset_to_data() const
{
	return this->offset_to_data;
}

DWORD ResourceData::get_size() const
{
	return this->size;
}

