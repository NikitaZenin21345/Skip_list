#pragma once
#include "random_number.h"

template<typename value_type>
class user_class
{
	value_type value;
public:
	user_class(value_type value_) :value(value_) {}
	bool operator<(const user_class other) const
	{
		return value < other.value;
	}
	bool operator==(user_class other) const
	{
		return value == other.value;
	}
	value_type get_value() const
	{
		return value;
	}
};

inline std::vector<double> get_random_vector(size_t size) 
{
	std::vector<double> vec;
	for(size_t index = 0 ;index!=size; index++)
	{
		vec.push_back(impl::get_random_number(0.0, 10.0));
	}
	return vec;
}