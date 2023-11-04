#pragma once
#include <concepts>
#include <type_traits>

namespace skip_list_condition
{
	template <class Compare, class value_type >
	concept is_compare = std::is_copy_constructible_v<Compare> && requires(Compare predicate,
		const Compare const_predicate, const  value_type value_first, const value_type value_second)
	{
		{predicate.operator()(value_first, value_second)} -> std::same_as<bool>;
		{const_predicate.operator()(value_first, value_second)} -> std::same_as<bool>;
	};


}
