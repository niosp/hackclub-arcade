#pragma once

#include <memory>
#include <windows.h>
#include <vector>

class ResourceDirectory;
class ResourceData;

class ResourceDirectoryEntry
{
public:
	ResourceDirectoryEntry(DWORD p_name, DWORD p_offset_to_data);
	DWORD get_name_is_string() const;
	DWORD get_data_is_directory() const;
	DWORD get_offset_to_directory() const;
	DWORD get_name_offset() const;
	DWORD get_name() const; // <- actually return RVA to name
	DWORD get_offset_to_data() const;
	std::shared_ptr<ResourceData> resource_data;
	void insert_nested_directory(std::shared_ptr<ResourceDirectory> p_directory) const;
	std::shared_ptr<std::vector<std::shared_ptr<ResourceDirectory>>> get_nested_directories() const;
	std::shared_ptr<ResourceData> get_stored_resource_data() const;
	void insert_resource_data(std::shared_ptr<ResourceData> p_resource_data);
private:
	DWORD name;
	DWORD offset_to_data;
	std::shared_ptr<std::vector<std::shared_ptr<ResourceDirectory>>> nested_directories;
};
