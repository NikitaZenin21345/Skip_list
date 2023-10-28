#pragma once
#include <exception>

class error_dereferencing_end : public std::exception
{
public:

	char const* what() const override
	{
		return "dereference end";
	}
};
