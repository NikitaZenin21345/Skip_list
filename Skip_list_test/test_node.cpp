#include "pch.h"
#include "Skip_list.h"
#include "user_class.h"

class SkipListNodeTest : public ::testing::Test {
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

TEST_F(SkipListNodeTest, Constructor) {
	constexpr size_t max_size = 16;
	using Node = skip_list_space::node<int, int, max_size>;
	Node default_node;
	EXPECT_THROW(default_node.next(), std::out_of_range);
	EXPECT_THROW(default_node.prev(), std::out_of_range);
	EXPECT_THROW(auto test_value = Node(std::pair(1, 2), max_size + 1), std::out_of_range);
	auto first_node = Node(std::pair(1, 2), max_size - 1);
	auto second_node(first_node);
	EXPECT_TRUE(first_node.get_key() == second_node.get_key());
	EXPECT_TRUE(first_node.get_level() == second_node.get_level());
	EXPECT_TRUE(first_node.get_value() == second_node.get_value());
	auto move_node(std::move(first_node));
	EXPECT_FALSE(first_node.is_valid());
}

TEST_F(SkipListNodeTest, NodeOutOfRange) {
	constexpr size_t max_size = 16;
	using Node = skip_list_space::node<user_class<int>, int, max_size>;
	Node default_node;
	EXPECT_THROW(default_node.next(), std::out_of_range);
	EXPECT_THROW(default_node.prev(), std::out_of_range);
	EXPECT_THROW(auto test_value = Node(std::pair(user_class(1), 2), max_size + 1), std::out_of_range);
	auto first_node = Node(std::pair(user_class(1), 2), max_size - 1);
	auto second_node(first_node);
	EXPECT_TRUE(first_node.get_key() == second_node.get_key());
	EXPECT_TRUE(first_node.get_level() == second_node.get_level());
	EXPECT_TRUE(first_node.get_value() == second_node.get_value());
	auto move_node(std::move(first_node));
	EXPECT_FALSE(first_node.is_valid());
}

TEST_F(SkipListNodeTest, NodeOperationSetGet) {
	constexpr size_t max_size = 16;
	using Node = skip_list_space::node<int, int, max_size>;
	auto first_node = Node(std::pair(1, 2), max_size - 1);
	auto second_node = Node(std::pair(2, 3), max_size - 1);
	auto third_node = Node(std::pair(3, 4), max_size - 1);
	static_assert(std::is_convertible_v<const std::pair<double,double >, std::pair<double, double >>);
	skip_list_space::node<int, int, max_size>::bind_node(&first_node, &second_node, max_size);
	EXPECT_TRUE(*first_node.next() == second_node);
	EXPECT_TRUE( *second_node.prev() == first_node);
	for(size_t index = 0; index < max_size; ++index)
	{
		EXPECT_TRUE(*second_node.get_left_node(index) == first_node);
		EXPECT_TRUE(*first_node.get_right_node(index) == second_node);
	}
	for (size_t index = 0; index < max_size ; ++index)
	{
		second_node.set_right_node(index, &third_node);
		EXPECT_TRUE(*second_node.get_right_node(index) == third_node);
	}
}
