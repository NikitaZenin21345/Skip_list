#pragma once
#include "random_number.h"

template<typename value_type>
class user_class
{
	value_type value{};
	value_type* ptr = nullptr;
public:
	user_class() = default;
	user_class(value_type value_) :value(value_)
	{
		ptr = new value_type[10](value_);
	}

	user_class(const user_class& other) noexcept
	{
		value = other.value;
		if(ptr != nullptr)
		{
			ptr = new value_type[10];
			memcpy(ptr, other.ptr, 10 * sizeof(value_type));
		}
	}

	user_class(user_class&& other) noexcept
	{
		value = std::move(other.value);
		ptr = other.ptr;
		other.ptr = nullptr;
	}

	user_class& operator=(user_class&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		value = std::move(other.value);
		std::swap(ptr, other.ptr);
		return *this;
	}

	user_class& operator=(const user_class& other) noexcept
	{
		if(this == &other)
		{
			return *this;
		}
		delete[] ptr;
		ptr = new value_type[10];
		value = other.value;
		memcpy(ptr, other.ptr, 10 * sizeof(value_type));
		return *this;
	}

	~user_class()
	{
		delete[] ptr;
	}

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