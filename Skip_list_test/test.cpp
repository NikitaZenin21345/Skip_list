#include "pch.h"
#include <iostream>
#include "Skip_list.h"
#include <crtdbg.h> 
#include "test_class.h"

class SkipListTest : public ::testing::Test {
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

TEST_F(SkipListTest, InsertElements) {
	auto list = skip_list_space::skip_list<int, int>();
	list.insert(std::pair<int, int>(0, 100));
	
}

TEST_F(SkipListTest, EraseOnceListElement) {
	auto list = skip_list_space::skip_list<user_class<int>, int>();
	list.insert(std::pair<user_class<int>, int>({ 0 }, 100));
	list.erase(0);
}

TEST_F(SkipListTest, FindElement) {
	using list_type = skip_list_space::skip_list<user_class<double>, int>;
	auto list = list_type();
	size_t size = 10;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair<double, int>(rand_vec[index], index));
	}
	auto finded_element = list.find(user_class(rand_vec[4]));
	EXPECT_TRUE((*finded_element).second == 4);
	auto const_finded_element = list.find(user_class(rand_vec[7]));
}

TEST_F(SkipListTest, Loop) {
	auto list = skip_list_space::skip_list<int, int, std::greater<>>(std::greater<>());
	size_t size = 200;
	for (int i = 1; i <= size; ++i)
	{
		list.insert(std::pair<int, int>(i, size - i));
	}
	int index = 0;
	for (const auto& val : list)
	{
		EXPECT_TRUE(val.second ==  index);
		++index;
	}
}

TEST_F(SkipListTest, ReverseLoop)
{
	auto list = skip_list_space::skip_list<double, int>();
	const size_t size = 10;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair<double, int>(rand_vec[index], index ));
	}
	const auto const_list = list;
	std::sort(rand_vec.begin(), rand_vec.end());

	int index = size - 1;
	for(auto iter = list.rbegin(); iter != list.rend(); ++iter)
	{
		EXPECT_DOUBLE_EQ( (*iter).first, rand_vec[index]);
		--index;
	}

	for (auto iter = const_list.rbegin(); iter != const_list.rend(); ++iter)
	{
		//(*iter).second = 0;
	}
}

TEST_F(SkipListTest, Iterator)
{
	using list_type = skip_list_space::skip_list<int, int>;
	auto list = list_type();
	size_t size = 200;
	for (int i = 0; i < size; ++i)
	{
		list.insert(std::pair(i, i));
	}
	const auto const_list = list;

	auto iter = list.begin();
	auto prev_next = --(++iter);
	EXPECT_TRUE(iter == prev_next);

	for (auto& [key, value] : list)
	{
		value = 0;
	}
	for (auto& [key, value] : list)
	{
		EXPECT_TRUE(value == 0);
	}

	for (auto iter = const_list.cbegin(); iter != const_list.cend(); ++iter)
	{
		//(*iter).second = 0;
	}
}

TEST_F(SkipListTest, Erase) {
	auto list = skip_list_space::skip_list<double, int, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair<double, int>(rand_vec[index], index));
	}

	auto iter = ++list.begin();
	auto erased_elem = *iter;
	EXPECT_TRUE(list.find(erased_elem.first) == iter);
	list.erase(iter);
	EXPECT_TRUE(list.find(erased_elem.first) == list.end());
	EXPECT_FALSE(iter == ++list.begin());
	EXPECT_TRUE(list.size() == size - 1);
}

TEST_F(SkipListTest, EraseRange) {
	auto list = skip_list_space::skip_list<double, int, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair<double, int>(rand_vec[index], index));
	}
	std::sort(rand_vec.begin(), rand_vec.end(), std::greater());

	auto iter = list.begin();
	for (size_t index = 0; index < size / 2; ++index)
	{
		++iter;
	}
	list.erase(list.begin(), iter);
	EXPECT_TRUE(list.size() == size / 2 - 1);
	for (size_t index = 0; index < size / 2; ++index)
	{
		EXPECT_TRUE(list.find(rand_vec[index]) == list.end());
	}
}

TEST_F(SkipListTest, EraseKey) {
	auto list = skip_list_space::skip_list<double, user_class<int>, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], user_class<int>(index)));
	}
	std::sort(rand_vec.begin(), rand_vec.end(), std::greater());

	list.erase(rand_vec[size / 2]);
	EXPECT_TRUE(list.size() == size - 1);
	EXPECT_TRUE(list.find(rand_vec[size / 2]) == list.end());
	list.erase(rand_vec[0]);
	EXPECT_TRUE(list.size() == size - 2);
	EXPECT_TRUE(list.find(rand_vec[0]) == list.end());
}

TEST_F(SkipListTest, Swap) {
	auto list = skip_list_space::skip_list<double, user_class<size_t>, std::greater<>>(std::greater<>());
	auto other_list = skip_list_space::skip_list<double, user_class<size_t>, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], user_class<size_t>(index)));
		other_list.insert(std::pair(rand_vec[index], user_class<size_t>(size - index)));
	}
	list.swap(other_list);
}

TEST_F(SkipListTest, OperatorEqual) {
	auto list = skip_list_space::skip_list<double, user_class<int>, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (int index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], user_class(index)));
	}
	auto other_list = list;
	EXPECT_TRUE(other_list == list);
}

TEST_F(SkipListTest, OperatorBraket) {
	auto list = skip_list_space::skip_list<int, user_class<int> , std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, 100, 1000);
	for (int index = 0; index < size; ++index)
	{
		list.insert(std::pair(index, user_class<int>(index)));
	}

	auto new_value = user_class(50);
	auto key = 50;
	list[key] = new_value;
	EXPECT_TRUE((*list.find(key)).second == new_value);
	list.erase(key);
	auto found_element = list.find(key);
	EXPECT_TRUE(found_element == list.end());
	list[key] = new_value;
	EXPECT_TRUE((*list.find(key)).second == new_value);
}

TEST_F(SkipListTest, At) {
	auto list = skip_list_space::skip_list<size_t, double, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(index, rand_vec[index]));
	}
	[[maybe_unused]] const auto const_list = list;

	double new_value = 20.0;
	auto key = 50;
	list.at(key) = new_value;
	EXPECT_DOUBLE_EQ((*list.find(key)).second, new_value);
	static_assert(std::is_same_v<decltype(const_list.at(key)),const double&>);
}

TEST_F(SkipListTest, Clear) {
	auto list = skip_list_space::skip_list<double, user_class<size_t>, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], user_class(index)));
	}
	list.clear();
	EXPECT_TRUE(list.size() == 0);
	for (size_t index = 0; index < size; ++index)
	{
		EXPECT_TRUE(list.find(rand_vec[index]) == list.end());
	}
}