#include "pch.h"
#include <crtdbg.h> 
#include "Skip_list.h"
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

TEST_F(SkipListTest, EmptyList) {
	auto list = skip_list_space::skip_list<size_t, size_t>();
	const auto other_list = skip_list_space::skip_list<size_t, size_t>();
	const auto found_element = list.find(0);
	EXPECT_TRUE(found_element == list.end());
	EXPECT_TRUE(list.erase(0) == 0);
	list.erase(list.begin(), list.end());
	EXPECT_TRUE(list.empty());
	list.clear();
	EXPECT_TRUE(list.empty());
	EXPECT_FALSE(list == other_list);
}

TEST_F(SkipListTest, InsertElements) {
	auto list = skip_list_space::skip_list<size_t, size_t>();
	auto pair_first = list.insert(std::pair(0, 100));
	EXPECT_TRUE(pair_first.second);
	auto found_element = list.find(0);
	EXPECT_TRUE((*found_element).second == 100);
	EXPECT_TRUE(list.size() == 1);
	list.erase(0);
	EXPECT_TRUE(list.empty());
	list[1] = 101;
	auto pair = list.insert(std::pair(1, 100));
	EXPECT_FALSE(pair.second);
	list.erase(list.begin(), list.end());
	EXPECT_TRUE(list.empty());
}

TEST_F(SkipListTest, EraseOnceListElement) {
	auto list = skip_list_space::skip_list<user_class<size_t>, size_t>();
	auto element = std::pair<user_class<size_t>, size_t>({ 0 }, 100);
	list.insert(std::move(element));
	EXPECT_TRUE(&(list = list) == &list);
	list.erase(0);
	const auto found_element = list.find(0);
	EXPECT_TRUE(found_element == list.end());
	list.erase(0);
}

TEST_F(SkipListTest, Constructor) {
	skip_list_space::skip_list<user_class<int>, size_t, std::less<>, 20> list;
	list.insert(std::pair<user_class<int>, size_t>({ 0 }, 100));
	list.insert(std::pair<user_class<int>, size_t>({ 1 }, 101));
	list.insert(std::pair<user_class<int>, size_t>({ 2 }, 102));
	EXPECT_TRUE(&(list = list) == &list);
	auto copy_list(list);
	auto move_list(std::move(list));
	EXPECT_TRUE(copy_list == move_list);
	move_list[user_class(1)] = 103;
	copy_list = move_list;
	EXPECT_TRUE(copy_list == move_list);
}

TEST_F(SkipListTest, UserCmp) {
	static_assert(skip_list_space::is_compare<user_cmp<size_t>, size_t>);
	using list_type = skip_list_space::skip_list<size_t, size_t, user_cmp<size_t>>;
	auto list = list_type();
	const size_t size = 100;
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(index, index));
	}
	auto found_element = list.begin();
	EXPECT_TRUE((*found_element).second == 99);
	auto last_found_element = --list.end();
	EXPECT_TRUE((*last_found_element).second == 0);
}

TEST_F(SkipListTest, FindElement) {
	using list_type = skip_list_space::skip_list<user_class<double>, size_t>;
	auto list = list_type();
	constexpr size_t size = 10;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);

	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], index));
	}
	auto found_element = list.find(user_class(rand_vec[4]));
	EXPECT_TRUE((*found_element).second == 4);
	const auto const_list = list;
	auto const_found_element = const_list.find(user_class(rand_vec[7]));
	/*(*const_found_element).second = 5;*/
}

TEST_F(SkipListTest, Loop) {
	auto list = skip_list_space::skip_list<size_t, size_t, std::greater<>>(std::greater<>());
	const size_t size = 200;
	for (size_t i = 1; i <= size; ++i)
	{
		list.insert(std::pair(i, size - i));
	}
	size_t index = 0;
	for (const auto& value : list)
	{
		EXPECT_TRUE(value.second == index);
		++index;
	}
}

TEST_F(SkipListTest, ReverseLoop)
{
	auto list = skip_list_space::skip_list<double, size_t>();
	const size_t size = 10;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], index));
	}
	const auto const_list = list;
	std::sort(rand_vec.begin(), rand_vec.end());

	size_t index = size - 1;
	for (auto list_iterator = list.rbegin(); list_iterator != list.rend(); ++list_iterator)
	{
		EXPECT_DOUBLE_EQ((*list_iterator).first, rand_vec[index]);
		--index;
	}
	index = size - 1;
	for (auto list_iterator = const_list.rbegin(); list_iterator != const_list.rend(); ++list_iterator)
	{
		EXPECT_DOUBLE_EQ((*list_iterator).first, rand_vec[index]);
		--index;
		//(*list_iterator).second = 0;
	}
	auto list_iterator = list.rbegin();
}

TEST_F(SkipListTest, Iterator)
{
	using list_type = skip_list_space::skip_list<size_t, size_t>;
	auto list = list_type();
	size_t size = 200;
	for (size_t i = 0; i < size; ++i)
	{
		list.insert(std::pair(i, i));
	}
	const auto const_list = list;

	auto list_iterator = list.begin();
	auto prev_next = --(++list_iterator);
	EXPECT_TRUE(list_iterator == prev_next);

	for (auto& [key, value] : list)
	{
		value = 0;
	}
	for (auto& [key, value] : list)
	{
		EXPECT_TRUE(value == 0);
	}
	size_t index = 0;
	for (auto list_iterator_ = const_list.cbegin(); list_iterator_ != const_list.cend(); ++list_iterator_)
	{
		EXPECT_TRUE((*list_iterator_).second == index);
		++index;
		//(*list_iterator).second = 0;
	}
	list_iterator = list.end();
	EXPECT_THROW(++list_iterator, std::out_of_range);
	EXPECT_THROW(*list_iterator, error_dereferencing_end);
	list_type::iterator iterator_first = list_iterator;
	list_type::const_iterator const_iterator = iterator_first;
	//list_type::iterator iterator_third = const_iterator;
}

TEST_F(SkipListTest, Erase) {
	auto list = skip_list_space::skip_list<double, size_t, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], index));
	}
	static_assert(std::is_convertible_v < std::pair<double, size_t>, const std::pair<const double, size_t >>);
	auto list_iterator = ++list.begin();
	auto erased_elem = *list_iterator;
	EXPECT_TRUE(list.find(erased_elem.first) == list_iterator);
	list.erase(list_iterator);
	EXPECT_TRUE(list.find(erased_elem.first) == list.end());
	EXPECT_FALSE(list_iterator == ++list.begin());
	EXPECT_TRUE(list.size() == size - 1);
	list.erase(list.begin(), list.end());
	EXPECT_TRUE(list.empty());
}

TEST_F(SkipListTest, EraseRange) {
	auto list = skip_list_space::skip_list<double, size_t, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair<const double, size_t>(rand_vec[index], index));
	}
	std::sort(rand_vec.begin(), rand_vec.end(), std::greater());

	auto list_iterator = list.begin();
	for (size_t index = 0; index < size / 2; ++index)
	{
		++list_iterator;
	}
	list.erase(list.begin(), list_iterator);
	EXPECT_TRUE(list.size() == size / 2);
	for (size_t index = 0; index < size / 2; ++index)
	{
		EXPECT_TRUE(list.find(rand_vec[index]) == list.end());
	}
}

TEST_F(SkipListTest, EraseKey) {
	auto list = skip_list_space::skip_list<double, user_class<size_t>, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], user_class(index)));
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
		list.insert(std::pair(rand_vec[index], user_class(index)));
		other_list.insert(std::pair(rand_vec[index], user_class(size - index)));
	}
	list.swap(other_list);
	for (size_t index = 0; index < size; ++index)
	{
		EXPECT_TRUE(list.at(rand_vec[index]) == user_class(size - index));
		EXPECT_TRUE(other_list.at(rand_vec[index]) == user_class(index));
	}
}

TEST_F(SkipListTest, Concept)
{
	static_assert(!skip_list_space::is_compare<bad_cmp<int>, int>);
	static_assert(!skip_list_space::valid_Value<bad_value<int>>);
	static_assert(!skip_list_space::valid_Key<bad_value<int>>);
}

TEST_F(SkipListTest, OperatorEqual) {
	auto list = skip_list_space::skip_list<double, user_class<size_t>, std::greater<>>(std::greater<>());
	const size_t size = 100;
	auto rand_vec = get_random_vector<double>(size, -10.0, 10.0);
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(rand_vec[index], user_class(index)));
	}
	auto other_list = list;
	EXPECT_TRUE(other_list == list);
	list[rand_vec[4]] = user_class<size_t>(5);
	EXPECT_TRUE(other_list != list);
}

TEST_F(SkipListTest, OperatorBraket) {
	auto list = skip_list_space::skip_list<int, user_class<int>, std::greater<>>(std::greater<>());
	const int size = 100;
	auto rand_vec = get_random_vector<double>(size, 100, 1000);
	for (int index = 0; index < size; ++index)
	{
		list.insert(std::pair(index, user_class(index)));
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
	static_assert(std::is_same_v<decltype(const_list.at(key)), const double&>);
	EXPECT_THROW(list.at(size), std::out_of_range);
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
	EXPECT_TRUE(list.empty());
	for (size_t index = 0; index < size; ++index)
	{
		EXPECT_TRUE(list.find(rand_vec[index]) == list.end());
	}
}

TEST_F(SkipListTest, ContainString) {
	auto list = skip_list_space::skip_list<size_t, std::string>();
	auto other_list = skip_list_space::skip_list<size_t, std::string>();
	const size_t size = 100;
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(index, "string number: " + std::to_string(index)));
		other_list.insert(std::pair(index, "other string number: " + std::to_string(size - index)));
	}
	for (size_t index = 0; index < size; ++index)
	{
		EXPECT_TRUE((*list.find(index)).second == "string number: " + std::to_string(index));
	}
	list[0] = "abc";
	EXPECT_TRUE((*list.find(0)).second == "abc");
	list.erase(list.begin(), list.end());
	EXPECT_TRUE(list.empty());

	list = std::move(other_list);
	for (size_t index = 0; index < size; ++index)
	{
		EXPECT_TRUE((*list.find(index)).second == "other string number: " + std::to_string(size - index));
	}
	skip_list_space::skip_list new_list(std::move(list));
	for (size_t index = 0; index < size; ++index)
	{
		EXPECT_TRUE((*new_list.find(index)).second == "other string number: " + std::to_string(size - index));
	}
}

TEST_F(SkipListTest, ContainVector) {
	auto list = skip_list_space::skip_list<size_t, std::vector<double>>();
	auto other_list = skip_list_space::skip_list<size_t, std::vector<double>>();
	const size_t size = 100;
	for (size_t index = 0; index < size; ++index)
	{
		list.insert(std::pair(index, get_random_vector<double>(size, -10.0, 10.0)));
		other_list.insert(std::pair(index, get_random_vector<double>(size, -10.0, 10.0)));
	}
	auto new_value = get_random_vector<double>(size, -10.0, 10.0);
	other_list[0] = new_value;
	EXPECT_TRUE((*other_list.find(0)).second == new_value);
	list.erase(list.begin(), list.end());
	EXPECT_TRUE(list.empty());
	list[1] = new_value;
	list = std::move(other_list);
	EXPECT_TRUE(list[1] != new_value);
	EXPECT_TRUE(list[0] == new_value);
	skip_list_space::skip_list new_list(std::move(list));
	EXPECT_TRUE(new_list[0] == new_value);
	static_assert(std::is_convertible_v<std::pair<const size_t, size_t>, std::pair<size_t, size_t>>);
}

TEST_F(SkipListTest, SkipListSkipLists) {
	using skip_list = skip_list_space::skip_list<size_t, std::vector<size_t>>;
	skip_list_space::skip_list <size_t, skip_list_space::skip_list < size_t, std::vector<size_t>>> list
	{
	  std::make_pair(1, skip_list()),
	  std::make_pair(2, skip_list()),
	  std::make_pair(3, skip_list())
	};
	const size_t size = 3;
	for (size_t index = 0; index < size - 1; ++index)
	{
		list[index].insert(std::pair(index, std::vector{ index, index, index }));
	}
	for (size_t index = 0; index < size - 1; ++index)
	{
		auto cmp_vector = std::vector{ index, index, index };
		auto lIst_element = (*list.find(index)).second;
		EXPECT_TRUE((*lIst_element.find(index)).second == cmp_vector);
	}
	list.erase(list.find(1));
	EXPECT_TRUE(list.find(1) == list.end());
}

TEST_F(SkipListTest, ComparisonListsAnyMaxLevel) {
	skip_list_space::skip_list<user_class<int>, size_t, std::less<>, 20> list1
	{
	  std::make_pair(user_class<int>(1), 101),
	  std::make_pair(user_class<int>(2), 102),
	  std::make_pair(user_class<int>(3), 103)
	};
	skip_list_space::skip_list<user_class<int>, size_t, std::less<>, 21> list2
	{
		std::make_pair(user_class<int>(1), 101),
		std::make_pair(user_class<int>(2), 102),
		std::make_pair(user_class<int>(3), 103)
	};
	EXPECT_TRUE(list1 == list2);
	list2[user_class<int>(2)] = 104;
	EXPECT_TRUE(list1 != list2);
}

