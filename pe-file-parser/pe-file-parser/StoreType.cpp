#include "StoreType.hpp"

StoreType::StoreType(const WORD p_type)
{
	this->type = p_type;
}

WORD StoreType::get_type() const
{
	return this->type;
}

