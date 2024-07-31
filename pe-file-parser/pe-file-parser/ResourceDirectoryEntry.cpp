#include "ResourceDirectoryEntry.hpp"
#include "ResourceDirectory.hpp"

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

void ResourceDirectoryEntry::insert_nested_directory(std::shared_ptr<ResourceDirectory> p_directory) const
{
	this->nested_directories->emplace_back(p_directory);
}

std::shared_ptr<std::vector<std::shared_ptr<ResourceDirectory>>> ResourceDirectoryEntry::get_nested_directories() const
{
	return this->nested_directories;
}

DWORD ResourceDirectoryEntry::get_offset_to_directory() const
{
	return (this->offset_to_data & 0x7FFFFFFF);
}







