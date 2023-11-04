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
template<typename type>
std::vector<type> get_random_vector(size_t size,const type low_bound,const type up_bound)
{
	std::vector<type> vec;
	for(size_t index = 0 ;index!=size; index++)
	{
		vec.push_back(impl::get_random_number<type>(low_bound, up_bound));
	}
	return vec;
}