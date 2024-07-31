#pragma once

#include <windows.h>

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
private:
	DWORD name;
	DWORD offset_to_data;
};