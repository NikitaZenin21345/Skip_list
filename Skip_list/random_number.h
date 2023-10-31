#pragma once
#include <random>

namespace impl
{
	constexpr double p = 1 / 2.;
	constexpr double min = 0.0;
	constexpr double max = 1.0;
	inline double get_random_number(const double min_lvl,const double max_lvl) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> distribution(min_lvl, max_lvl);
		return distribution(gen);
	}
	
}


inline size_t random_level(const size_t max_lvl)
{
	size_t lvl = 1;
	while (impl::get_random_number(impl::min, impl::max) < impl::p && lvl < max_lvl )
	{
		lvl++;
	}
	return lvl;
}
