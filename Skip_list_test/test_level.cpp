#include "pch.h"
#include "Skip_list.h"

TEST(LevelTest, Boarder)
{
	skip_list_space::Level<64> level(16);
	EXPECT_THROW(skip_list_space::Level<64> bad_level(65), std::out_of_range);
	const size_t value = 64;
	size_t other_value = 0;
	level = value;
	EXPECT_THROW(++level, std::out_of_range);
	EXPECT_THROW(level += 1, std::out_of_range);
	EXPECT_THROW(auto test_value = level + value, std::out_of_range);
	level = other_value;
	EXPECT_THROW(--level, std::out_of_range);
	EXPECT_THROW(level -= 1, std::out_of_range);
	EXPECT_THROW(auto test_value = level - value, std::out_of_range);
}

TEST(LevelTest, ArithmeticOperation)
{
	size_t start_value = 16;
	size_t second_start_value = 48;
	skip_list_space::Level<64> level_first(start_value);
	skip_list_space::Level<64> level_second(second_start_value);
	auto value = level_first + level_second;
	EXPECT_TRUE(value == static_cast<size_t>(64));
	value = level_second - level_first;
	auto sum_value = level_second + level_first;
	EXPECT_TRUE(value == (second_start_value - start_value));
	EXPECT_TRUE(sum_value == (second_start_value + start_value));
	EXPECT_THROW(auto test_value = level_first - level_second, std::out_of_range);
	EXPECT_TRUE(--level_first == --start_value);
	EXPECT_TRUE(level_first-- == start_value--);
	EXPECT_TRUE(++level_first == ++start_value);
	EXPECT_TRUE(level_first++ == start_value++);
}

TEST(LevelTest, CompareOperation)
{
	size_t start_value = 16;
	size_t second_start_value = 48;
	skip_list_space::Level<64> level_first(start_value);
	EXPECT_TRUE(level_first == start_value);
	EXPECT_TRUE(level_first <= start_value);
	EXPECT_TRUE(level_first > skip_list_space::Level<64>(15));
	EXPECT_TRUE(level_first < second_start_value);
	EXPECT_TRUE(level_first >= start_value);
	
}