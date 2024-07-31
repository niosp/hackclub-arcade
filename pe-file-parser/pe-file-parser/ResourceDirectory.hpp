#pragma once

#include <windows.h>

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
private:
	DWORD characteristics;
	DWORD time_date_stamp;
	WORD major_version;
	WORD minor_version;
	WORD number_of_named_entries;
	WORD number_of_id_entries;
};
