#pragma once
#include "random_number.h"

template<typename value_type>
class user_class
{
	value_type value{};
	value_type* ptr = nullptr;
	inline static size_t copy_counter = 0;
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
		copy_counter += 1;
	}

	user_class(user_class&& other) noexcept
	{
		value = std::move_if_noexcept(other.value);
		ptr = other.ptr;
		other.ptr = nullptr;
		copy_counter += 1;
	}

	user_class& operator=(user_class&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		value = std::move_if_noexcept(other.value);
		std::swap(ptr, other.ptr);
		copy_counter += 1;
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
		copy_counter++;
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

	static size_t get_copy_counter()
	{
		return copy_counter;
	}
};

template<typename type>
std::vector<type> get_random_vector(size_t size,const type low_bound,const type up_bound)
{
	std::vector<type> vec;
	std::random_device rd;
	std::mt19937 gen(rd());
	for(size_t index = 0 ;index!=size; index++)
	{
		vec.push_back(random_tools::impl::get_random_number<type>(low_bound, up_bound, gen));
	}
	return vec;
}

template<typename Value>
class user_cmp
{
public:
	bool operator()(const Value& value_first,const Value& value_second) const
	{
		return value_second < value_first;
	}
};

template<typename Value>
class bad_cmp final
{
public:
	bad_cmp(const bad_cmp& other) = delete;
	int operator()(const Value& value_first, const Value& value_second) const
	{
		return value_second - value_first;
	}
};

template<typename Value>
class bad_value final
{
	int a{};
public:
	bad_value(const bad_value& other) = delete;
	bool operator==(const bad_value&) const= delete;
};