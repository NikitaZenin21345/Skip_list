#include "pch.h"
#include <iostream>
#include "Skip_list.h"
#include <crtdbg.h> 
#include "test_class.h"

class SkipListTestMem : public ::testing::Test {
protected:
	virtual void SetUp(void) {
		_CrtMemCheckpoint(&startup);
	}
	virtual void TearDown(void) {
		_CrtMemState teardown, diff;
		_CrtMemCheckpoint(&teardown);
		ASSERT_EQ(0, _CrtMemDifference(&diff, &startup, &teardown)) << "Memory leaks detected";
	}
	_CrtMemState startup;
};

TEST_F(SkipListTestMem, InsertElements) {
	auto list = skip_list<int, int, std::greater<>>(std::greater<>());
	for (int i = 0; i < 200; ++i)
	{
		list.insert(std::pair<int, int>(i, 100 + i));
	}
	int index = 0;
	for (const auto& a : list)
	{
		std::cout << a.second << std::endl;
	}
}

TEST_F(SkipListTestMem, EraseOnceListElement) {
	auto list = skip_list<user_class<int>, int>();
	list.insert(std::pair<user_class<int>, int>({ 0 }, 100));
	list.erase(0);
	std::cout << list.size() << std::endl;
}

TEST(SkipListTest, Loop) {
	auto list = skip_list<int, int>();
	list.insert(std::pair<int, int>(0, 100));
	list.erase(0);
	std::cout << list.size() << std::endl;
}

TEST(SkipListTest, ReverseLoop) {
	auto list = skip_list<double, int>();
	size_t size = 10;
	auto rand_vec = get_random_vector(size);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair<double, int>(rand_vec[index], index ));
	}
	//std::sort(rand_vec.begin(), rand_vec.end());
	for(auto [key, value] : list)
	{
		EXPECT_DOUBLE_EQ(key , rand_vec[value]);
	}
}