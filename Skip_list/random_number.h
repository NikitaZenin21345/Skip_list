#pragma once
#include <random>

namespace impl
{
	constexpr double p = 1 / 2.;
	inline double get_random_number() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> distribution(0.0, 1.0);
		return distribution(gen);
	}
	
}


inline size_t random_level(const size_t max_lvl)
{
	size_t lvl = 1;
	while (impl::get_random_number() < impl::p && lvl < max_lvl )
	{
		lvl++;
	}
	return lvl;
}
