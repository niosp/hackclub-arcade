#include "ResourceDirectoryEntry.hpp"

#include <bitset>

class ResourceDirectory;
class ResourceData;

ResourceDirectoryEntry::ResourceDirectoryEntry(DWORD p_name, DWORD p_offset_to_data)
{
	this->name = p_name;
	this->offset_to_data = p_offset_to_data;
	this->nested_directories = std::make_shared<std::vector<std::shared_ptr<ResourceDirectory>>>();
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

std::shared_ptr<ResourceData> ResourceDirectoryEntry::get_stored_resource_data() const
{
	return this->resource_data;
}

void ResourceDirectoryEntry::insert_resource_data(std::shared_ptr<ResourceData> p_resource_data)
{
	this->resource_data = p_resource_data;
}







