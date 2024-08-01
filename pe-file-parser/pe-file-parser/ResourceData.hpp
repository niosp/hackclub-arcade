#pragma once

#include <Windows.h>

class ResourceData
{
public:
	ResourceData(DWORD offset_to_data, DWORD size, DWORD code_page);
	DWORD get_offset_to_data() const;
	DWORD get_size() const;
	DWORD get_code_page() const;
private:
	DWORD offset_to_data;
	DWORD size;
	DWORD code_page;
};