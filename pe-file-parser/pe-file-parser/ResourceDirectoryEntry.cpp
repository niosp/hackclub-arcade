#include "ResourceDirectoryEntry.hpp"

#include <bitset>


ResourceDirectoryEntry::ResourceDirectoryEntry(DWORD p_name, DWORD p_offset_to_data)
{
	this->name = p_name;
	this->offset_to_data = p_offset_to_data;
}

DWORD ResourceDirectoryEntry::get_data_is_directory() const
{
	std::bitset<sizeof(DWORD)> bits(this->get_offset_to_data());
	
	return (this->offset_to_data >> 31) & 1;
}

DWORD ResourceDirectoryEntry::get_name_is_string() const
{
	std::bitset<sizeof(DWORD)> bits(this->get_offset_to_data());

	return (this->offset_to_data >> 31) & 1;
	
}

DWORD ResourceDirectoryEntry::get_name() const
{
	return this->name;
}

DWORD ResourceDirectoryEntry::get_name_offset() const
{
	return (this->name & 0x7FFFFFFF);
}

DWORD ResourceDirectoryEntry::get_offset_to_data() const
{
	return this->offset_to_data;
}

DWORD ResourceDirectoryEntry::get_offset_to_directory() const
{
	return (this->offset_to_data & 0x7FFFFFFF);
}







