#pragma once
#include <random>

namespace impl
{
	constexpr double p = 1 / 2.;
	constexpr double min = 0.0;
	constexpr double max = 1.0;
	template<typename type>
	double get_random_number(const type min_lvl,const type max_lvl) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<type> distribution(min_lvl, max_lvl);
		return distribution(gen);
	}
	
}


inline size_t random_level(const size_t max_lvl)
{
	size_t lvl = 1;
	while (impl::get_random_number<double>(impl::min, impl::max) < impl::p && lvl < max_lvl )
	{
		lvl++;
	}
	return lvl;
}
