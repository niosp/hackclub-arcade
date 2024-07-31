#include "ResourceDirectory.hpp"

#include <windows.h>

ResourceDirectory::ResourceDirectory(DWORD p_characteristics, DWORD p_time_date_stamp, WORD p_major_version, WORD p_minor_version, WORD p_number_of_named_entries, WORD p_number_of_id_entries)
{
	this->characteristics = p_characteristics;
	this->time_date_stamp = p_time_date_stamp;
	this->major_version = p_major_version;
	this->minor_version = p_minor_version;
	this->number_of_named_entries = p_number_of_named_entries;
	this->number_of_id_entries = p_number_of_id_entries;
}

DWORD ResourceDirectory::get_characteristics() const
{
	return this->characteristics;
}

DWORD ResourceDirectory::get_time_date_stamp() const
{
	return this->time_date_stamp;
}

WORD ResourceDirectory::get_major_version() const
{
	return this->major_version;
}

WORD ResourceDirectory::get_minor_version() const
{
	return this->minor_version;
}

WORD ResourceDirectory::get_number_of_named_entries() const
{
	return this->number_of_named_entries;
}

WORD ResourceDirectory::get_number_of_id_entries() const
{
	return this->number_of_id_entries;
}