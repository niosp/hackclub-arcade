#pragma once

#include <memory>
#include <vector>
#include <windows.h>

class ResourceDirectoryEntry;

class ResourceDirectory
{
public:
	ResourceDirectory(DWORD p_characteristics, DWORD p_time_date_stamp, WORD p_major_version, WORD p_minor_version, WORD p_number_of_named_entries, WORD p_number_of_id_entries);
	DWORD get_characteristics() const;
	DWORD get_time_date_stamp() const;
	WORD get_major_version() const;
	WORD get_minor_version() const;
	WORD get_number_of_named_entries() const;
	WORD get_number_of_id_entries() const;
	void insert_directory_entry(std::shared_ptr<ResourceDirectoryEntry> p_entry) const;
	std::shared_ptr<std::vector<std::shared_ptr<ResourceDirectoryEntry>>> get_directory_entries();
private:
	DWORD characteristics;
	DWORD time_date_stamp;
	WORD major_version;
	WORD minor_version;
	WORD number_of_named_entries;
	WORD number_of_id_entries;
	std::shared_ptr<std::vector<std::shared_ptr<ResourceDirectoryEntry>>> resource_entries;
};
