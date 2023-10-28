#include "pch.h"
#include <iostream>
#include "Skip_list.h"

TEST(SkipListTest, ConstructList) {
	auto list = skip_list<int, int>(std::less<int>());
	for (int i = 0; i < 10; ++i)
	{
		list.insert(std::pair<int, int>(i, 100 + i));
	}
	std::cout << list.size();
}