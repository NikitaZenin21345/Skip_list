#include "pch.h"
#include <iostream>
#include "Skip_list.h"
#include <crtdbg.h> 

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

//TEST_F(SkipListTestMem, ConstructList) {
//	auto list = skip_list<int, int>(std::less<int>());
//	for (int i = 0; i < 1; ++i)
//	{
//		list.insert(std::pair<int, int>(i, 100 + i));
//	}
//	for (const auto& a : list)
//	{
//		std::cout << a.get_value() << std::endl;
//	}
//	/*for (auto a = list.begin(); a != list.end(); ++a) 
//	{
//		std::cout << a.operator*().get_value() << std::endl;
//	}*/
//	for (int i = 0; i < 1; ++i)
//	{
//		list.insert(std::pair<int, int>(i, 101 + i));
//	}
//	for (const auto& a: list)
//	{
//		std::cout << a.get_value() << std::endl;
//	}
//}

TEST_F(SkipListTestMem, InsertElement) {

	auto list = skip_list<int, int, std::greater<>>(std::greater<>());
	for (int i = 0; i < 1000; ++i)
	{
		list.insert(std::pair<int, int>(i, 100 + i));
	}
	//auto a = std::map<int,int>([](int a, int b) {return a > b; });
	/*list.insert(std::pair<int, int>(0, 100));
	list.erase(0);
	std::cout << list.size() << std::endl;*/
}

TEST_F(SkipListTestMem, EraseOnceListElement) {
	auto list = skip_list<int, int>();
	list.insert(std::pair<int, int>(0, 100));
	//list.erase(0);
	std::cout << list.size() << std::endl;
}

TEST(SkipListTest, Loop) {
	auto list = skip_list<int, int>();
	list.insert(std::pair<int, int>(0, 100));
	//list.erase(0);
	std::cout << list.size() << std::endl;
}