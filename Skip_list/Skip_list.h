#pragma once
#include <memory>
#include <utility>
#include <vector>
#include <memory>
#include <iterator>
#include <concepts>
#include <type_traits>
#include <initializer_list>
#include "random_number.h"
#include "skip_list_exception.h"

namespace skip_list_space
{
	template<size_t Max_level>
	class Level final
	{
		size_t size_{};
		void max_boarder_check(const size_t other_size) const
		{
			if (size_ + other_size > Max_level)
			{
				throw std::out_of_range("level is larger than max");
			}
		}

		void min_boarder_check(const size_t other_size) const
		{
			if (size_ < other_size)
			{
				throw std::out_of_range("level is less than minimum");
			}
		}
	public:
		
		Level() = default;
		Level(const size_t size)
		{
			max_boarder_check(size);
			size_ = size;
		}

		[[nodiscard]] size_t get_size() const noexcept { return size_; }
		void set_size(size_t size) noexcept { size_ = size; }
		operator size_t() const noexcept { return size_; }
		bool operator<(const size_t other) const noexcept { return size_ < other; }
		bool operator>(const size_t other) const noexcept { return size_ > other; }
		bool operator<=(const size_t other) const noexcept { return size_ <= other; }
		bool operator>=(const size_t other) const noexcept { return size_ >= other; }
		bool operator==(const size_t other_size) const noexcept{ return size_ == other_size; }

		Level& operator=(const size_t other_size)
		{
			if(other_size > Max_level)
			{
				throw std::out_of_range("level is larger than max");
			}
			size_ = other_size;
			return *this;
		}

		Level& operator-=(const size_t other_size) 
		{
			min_boarder_check(other_size);
			size_ -= other_size;
			return *this;
		}

		Level& operator+=(const size_t other_size) 
		{
			max_boarder_check(other_size);
			size_ += other_size;
			return *this;
		}

		[[nodiscard]] Level operator-(const size_t other_size) const 
		{
			min_boarder_check(other_size);
			return Level(size_ - other_size);
		}

		[[nodiscard]] Level operator+(const size_t other_size) const 
		{
			max_boarder_check(other_size);
			return Level(size_ + other_size);
		}

		Level& operator++() 
		{
			max_boarder_check(1);
			++size_;
			return *this;
		}

		Level operator++(int)
		{
			max_boarder_check(1);
			auto prev = *this;
			++size_;
			return prev;
		}

		Level& operator--() 
		{
			min_boarder_check(1);
			--size_;
			return *this;
		}

		Level operator--(int)
		{
			min_boarder_check(1);
			auto prev = *this;
			--size_;
			return prev;
		}
	};

	template <class Compare, class value_type >
	concept is_compare = std::is_copy_constructible_v<Compare> && requires(Compare predicate,
		const Compare const_predicate, const  value_type value_first, const value_type value_second)
	{
		{predicate.operator()(value_first, value_second)} -> std::same_as<bool>;
		{const_predicate.operator()(value_first, value_second)} -> std::same_as<bool>;
	};

	template <class Value>
	concept valid_Value = std::is_copy_constructible_v<Value>;

	template <class Value>
	concept comparable_value = requires(const Value const_value, Value value, const Value const_other, Value other)
	{
		{value == const_value} -> std::same_as<bool>;
		{value == other} -> std::same_as<bool>;
		{const_other == other} -> std::same_as<bool>;
		{const_other == const_value} -> std::same_as<bool>;
	};

	template <class Key>
	concept valid_Key = std::is_copy_constructible_v<Key> &&
		requires(Key key, const Key const_key,Key other_key,const Key const_other)
	{
		{key == other_key} -> std::same_as<bool>;
		{const_key == other_key} -> std::same_as<bool>;
		{other_key == const_key} -> std::same_as<bool>;
		{const_other == const_key} -> std::same_as<bool>;
	};

	template <typename Key,
		typename Value, size_t Max_level, typename Alloc = std::allocator<std::pair<const Key, Value>>>
	class node final
	{
		std::vector<node*> right_nodes{};
		std::vector<node*> left_nodes{};
		std::pair<const Key, Value>* node_value = nullptr;
		Level<Max_level> level{};
		Alloc allocator{};
		bool valid = false;

		void init_node_value(std::pair<const Key, Value>* value, Level<Max_level> level_)
		{
			set_null_neighbours(level_);
			if (node_value != nullptr)
			{
				allocator.deallocate(node_value, sizeof(*value));
			}
			node_value = allocator.allocate(sizeof(*value));
			node_value->first = value->first;
			node_value->second = value->second;
		}

		void init_node_value(std::vector<node*>&& right_nodes_, std::vector<node*>&& left_nodes_, std::pair<const Key, Value>* value) noexcept
		{
			left_nodes(std::move_if_noexcept(left_nodes_));
			right_nodes(std::move_if_noexcept(right_nodes_));
			node_value = value;
		}

		void set_null_neighbours(Level<Max_level> level_)
		{
			left_nodes.assign(level_, nullptr);
			right_nodes.assign(level_, nullptr);
		}

	public:
		node() = default;

		template<typename Pair>
			requires std::is_convertible_v<std::pair< Key, Value>, Pair>
		node( Pair&& node_value_, const Level<Max_level> level_, const Alloc& alloc = Alloc())
			: level(level_), allocator(alloc)
		{
			node_value = std::allocator_traits<Alloc>::allocate(allocator, 1);
			std::allocator_traits<Alloc>::construct(allocator, node_value, std::forward<Pair>(node_value_));
			set_null_neighbours(level_);
			valid = true;
		}

		node(const node& other) noexcept
			: level(other.level), allocator(other.allocator)
		{
			set_null_neighbours(other.level);
			node_value = std::allocator_traits<Alloc>::allocate(allocator, 1);
			std::allocator_traits<Alloc>::construct(allocator, node_value, other.node_value->first, other.node_value->second);
			valid = true;
		}

		node(node&& other) noexcept : right_nodes(std::move_if_noexcept(other.right_nodes)), left_nodes (std::move_if_noexcept(other.left_nodes)),
			node_value(other.node_value), level(std::move_if_noexcept(other.level)), allocator(std::move_if_noexcept(other.allocator))
		{
			other.node_value = nullptr;
			other.valid = false;
			valid = true;
		}

		void link_with_left_node(node* left_node, size_t index)
		{
			this->set_left_node(index, left_node);
			left_node->set_right_node(index, this);
		}

		node& operator=(const node& other)
		{
			if (this == &other)
			{
				return *this;
			}
			allocator = other.allocator;
			level = other.level;
			init_node_value(other.node_value, level);
			return *this;
		}

		node& operator=(node&& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}
			level = other.level;
			allocator = std::move_if_noexcept(other.allocator);
			init_node_value(other.right_nodes, other.left_nodes, other.node_value);
			other.node_value = nullptr;
			return *this;
		}

		~node()
		{
			if (node_value != nullptr)
			{
				std::allocator_traits<Alloc>::destroy(allocator, node_value);
				std::allocator_traits<Alloc>::deallocate(allocator, node_value, 1);
			}
		}

		void set_right_node(const size_t index, node* value_) 
		{
			if(this == value_){	return;	}
			right_nodes.at(index) = value_;
		}

		void set_left_node(const size_t index, node* value_) 
		{
			if (this == value_) { return; }
			left_nodes.at(index) = value_;
		}

		[[nodiscard]] bool is_valid() const noexcept
		{
			return valid;
		}

		static void bind_node(node* first, node* second, Level<Max_level> level_) noexcept
		{
			first->level = level_;
			second->level = level_;
			second->right_nodes.assign(level_, nullptr);
			second->left_nodes.assign(level_, first);
			first->left_nodes.assign(level_, nullptr);
			first->right_nodes.assign(level_, second);
		}

		decltype(auto) get_node_value() { return *node_value; }
		node* get_right_node(const size_t  index) noexcept { return right_nodes[index]; }
		node* get_left_node(const size_t index) noexcept { return left_nodes[index]; }
		node* next() { return right_nodes.at(0); }
		[[nodiscard]] const node* next() const { return right_nodes.at(0); }
		node* prev() { return left_nodes.at(0); }
		[[nodiscard]] const node* prev() const{ return left_nodes.at(0); }

		bool operator==(const node& other) const
		{
			return node_value->first == other.node_value->first && node_value->second == other.node_value->second;
		}

		[[nodiscard]] Level<Max_level> get_level() const noexcept { return level; }
		[[nodiscard]] const Key& get_key() const noexcept { return node_value->first; }
		[[nodiscard]] Value& get_value() noexcept { return node_value->second; }
	};

	template <bool IsConst, typename Key, typename Value, size_t Max_level = 10, typename Alloc = std::allocator<std::pair<const Key, Value>>>
	class node_iterator final
	{
		node<Key, Value, Max_level, Alloc>* list_begin;
		node<Key, Value, Max_level, Alloc>* list_end;
		node<Key, Value, Max_level, Alloc>* node_pointer;

		void out_of_range_check(node<Key, Value, Max_level, Alloc>* boundary) const
		{
			if (node_pointer == boundary)
			{
				throw std::out_of_range("out of range");
			}
		}
		
		friend node_iterator<!IsConst, Key, Value, Max_level, Alloc>;
	public:
		using value_type = std::pair<const Key, Value>;
		using reference = std::pair<const Key, Value>&;
		using pointer = std::pair<const Key, Value>*;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::bidirectional_iterator_tag;
		using condition_ref = std::conditional_t<IsConst, std::add_const_t<std::remove_reference_t<reference>>&, reference>;

		node_iterator(node<Key, Value, Max_level, Alloc>* begin_, node<Key, Value, Max_level, Alloc>* end_, node<Key, Value, Max_level, Alloc>* node_ptr)
			:list_begin(begin_), list_end(end_), node_pointer(node_ptr) {}

		template<bool Other_Const>
			requires !Other_Const || IsConst
		node_iterator(const node_iterator<Other_Const, Key, Value, Max_level, Alloc>& other)
		{
			list_begin = other.list_begin;
			list_end = other.list_end;
			node_pointer = other.node_pointer;
		}

		condition_ref operator*()
		{
			if (node_pointer == list_end)
			{
				throw error_dereferencing_end();
			}
			return node_pointer->get_node_value();
		}

		bool operator==(const node_iterator& other) const noexcept
		{
			return node_pointer == other.node_pointer;
		}

		bool operator!=(const node_iterator& other) const noexcept
		{
			return node_pointer != other.node_pointer;
		}

		decltype(auto) get_node_value()
		{
			return node_pointer;
		}

		decltype(auto) operator++()
		{
			out_of_range_check(list_end);
			node_pointer = node_pointer->next();
			return *this;
		}

		decltype(auto) operator++(int)
		{
			out_of_range_check(list_end);
			auto node = *this;
			++(*this);
			return node;
		}

		decltype(auto) operator--()
		{
			out_of_range_check(list_begin);
			node_pointer = node_pointer->prev();
			return *this;
		}
	};

	template <valid_Key Key,
		valid_Value Value,
		typename Compare = std::less<Key>,
		size_t Max_level = 10,
		typename Alloc = std::allocator<std::pair<const Key, Value>>, unsigned int Seed = 5489U>
		requires is_compare<Compare, Key>
	class skip_list final {

		node<Key, Value, Max_level, Alloc>* head = nullptr;
		node<Key, Value, Max_level, Alloc>* tail = nullptr;
		Compare compare;
		Alloc allocator;
		size_t list_size{};
		std::mt19937 random_number_generator{Seed};
		Level<Max_level> list_lvl{};

		template<typename Key_ = Key>
		[[nodiscard]] bool equal_key(const Key_& first, const Key_& second) const
		{
			return first == second;
		}

		template<typename Key_>
			requires std::is_same_v<Key, double> || std::is_same_v<Key, float>
		[[nodiscard]] bool equal_key(const Key_& first, const Key_& second) const
		{
			return std::fabs(first - second) < std::numeric_limits<Key_>::epsilon();
		}

		void insert_sorted_nodes(node<Key, Value, Max_level, Alloc>* nodes_head, node<Key, Value, Max_level, Alloc>* nodes_tail)
		{
			std::vector<node<Key, Value, Max_level, Alloc>*> array_no_linked_nodes;
			array_no_linked_nodes.assign(Max_level, head);
			for (auto inserted_node = nodes_head->next(); inserted_node != nodes_tail; inserted_node = inserted_node->next())
			{
				auto new_node = new node<Key, Value, Max_level, Alloc>(*inserted_node);
				for (Level<Max_level> index = 0; index < new_node->get_level(); ++index)
				{
					new_node->link_with_left_node(array_no_linked_nodes[index], index);
					array_no_linked_nodes[index] = new_node;
				}
			}
			for (size_t index = 0; index < Max_level; ++index)
			{
				tail->link_with_left_node(array_no_linked_nodes[index], index);
			}
		}

		decltype(auto) next_less_key_element(node<Key, Value,Max_level, Alloc>* node, int lvl_index,const Key& key) const
		{
			while (node->get_right_node(lvl_index) != tail &&
				compare(node->get_right_node(lvl_index)->get_key(), key))
			{
				node = node->get_right_node(lvl_index);
			}
			return node;
		}

		decltype(auto) search_key_storing_past_elements(const Key& key, const Level<Max_level> level)
		{
			std::vector<node<Key, Value, Max_level, Alloc>*> past_elements;
			past_elements.assign(Max_level, nullptr);
			auto node = head;
			for (int lvl_index = static_cast<int>(level.get_size()) - 1; lvl_index >= 0; --lvl_index)
			{
				node = next_less_key_element(node, lvl_index, key);
				past_elements[lvl_index] = node;
			}
			return past_elements;
		}

		[[nodiscard]] node<Key, Value, Max_level, Alloc>* search_key(const Key& key) const
		{
			auto node = head;
			for (int lvl_index = static_cast<int>(list_lvl.get_size()) - 1; lvl_index >= 0; --lvl_index)
			{
				node = next_less_key_element(node, lvl_index, key);
			}
			if (list_size != 0)
			{
				if (equal_key(node->next()->get_key(), key))
				{
					return node->next();
				}
			}
			return tail;
		}

		[[nodiscard]] Level<Max_level> find_max_lvl() const noexcept
		{
			Level<Max_level> max = 0;
			for (const_iterator list_node = cbegin(); list_node != cend(); ++list_node)
			{
				auto new_max = list_node.get_node_value()->get_level();
				if (new_max > max)
				{
					max = new_max;
				}
			}
			return max;
		}

		void delete_node(node<Key, Value, Max_level, Alloc>* del_node)
		{
			if (del_node != head && del_node != tail)
			{
				Level<Max_level> lvl = del_node->get_level();
				for (Level < Max_level> lvl_index = 0; lvl_index < lvl; ++lvl_index)
				{
					auto next_element = del_node->get_right_node(lvl_index);
					auto prev_element = del_node->get_left_node(lvl_index);
					next_element->link_with_left_node( prev_element, lvl_index);
				}
				list_size -= 1;
				list_lvl = find_max_lvl();
			}

			delete del_node;
		}

		void delete_list()
		{
			if(head == nullptr || tail == nullptr)
			{
				return;
			}
			auto del_node = head;
			auto next_node = head->next();
			if (next_node == tail)
			{
				delete head;
			}
			else
			{
				while (del_node != tail)
				{
					delete del_node;
					del_node = next_node;
					next_node = next_node->next();
				}
			}
			delete tail;
			list_size = 0;
			list_lvl = 0;
		}

		void init_head_and_tail()
		{
			head = new node<Key, Value, Max_level, Alloc>;
			tail = new node<Key, Value, Max_level, Alloc>;
			node<Key, Value, Max_level, Alloc>::bind_node(head, tail, Max_level);
		}

	public:
		using iterator = node_iterator<false, Key, Value, Max_level, Alloc>;
		using const_iterator = node_iterator<true, Key, Value, Max_level, Alloc>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using value_type = std::pair<const Key, Value>;
		using size_type = size_t;

		explicit skip_list(const Compare& comp = Compare(), const Alloc& alloc = Alloc()) : compare(comp), allocator(alloc)	{}

		explicit skip_list(const std::initializer_list<value_type>& list, const Compare& comp = Compare(), const Alloc& alloc = Alloc())
		: skip_list(comp, alloc)
		{
			init_head_and_tail();
			for(const auto& inserted_value : list)
			{
				insert(inserted_value);
			}
		}

		~skip_list()
		{
			delete_list();
		}

		skip_list(const skip_list& another) : compare(another.compare), allocator(another.allocator),
			list_size(another.list_size), list_lvl(another.list_lvl)
		{
			if (!another.empty())
			{
				init_head_and_tail();
				insert_sorted_nodes(another.head, another.tail);
			}
		}

		skip_list(skip_list&& another) noexcept : head(another.head), tail(another.tail),compare(std::move_if_noexcept(another.compare)),
			allocator(std::move_if_noexcept(another.allocator)),list_size(another.list_size), list_lvl(another.list_lvl)
		{
			another.list_lvl = 0;
			another.list_size = 0;
			another.head = nullptr;
			another.tail = nullptr;
		}

		skip_list& operator=(const skip_list& another) noexcept
		{
			if (this == &another)
			{
				return *this;
			}
			delete_list();
			compare = another.compare;
			allocator = another.allocator;
			list_lvl = another.list_lvl;
			list_size = another.list_size;
			if (!another.empty())
			{
				init_head_and_tail();
				insert_sorted_nodes(another.head, another.tail);
			}
			return *this;
		}

		skip_list& operator=(skip_list&& another) noexcept
		{
			if (this == &another)
			{
				return *this;
			}
			compare = std::move_if_noexcept(another.compare);
			allocator = std::move_if_noexcept(another.allocator);
			list_size = another.list_size;
			list_lvl = another.list_lvl;
			std::swap(head, another.head);
			std::swap(tail, another.tail);
			return *this;
		}

		iterator begin()
		{
			if(head == nullptr){return end();}
			return iterator(head, tail, head->next());
		}

		iterator end() { return iterator(head, tail, tail); }

		[[nodiscard]] const_iterator cbegin() const
		{
			if (head == nullptr){return cend();}
			return const_iterator(head, tail, head->next());
		}

		[[nodiscard]] const_iterator cend() const { return const_iterator(head, tail, tail); }

		[[nodiscard]] bool empty() const
		{
			return list_size == static_cast<size_t>(0);
		}

		[[nodiscard]] size_t size() const { return list_size; }

		Value& operator[](const Key& key)
			requires std::is_default_constructible_v<Value>
		{
			auto searched_key = search_key(key);
			if (searched_key == tail)
			{
				Value new_value{};
				auto new_node = insert(std::make_pair(key, new_value));
				return (*new_node.first).second;
			}
			return searched_key->get_value();
		}

		[[nodiscard]] const Value& at(const Key& key) const
		{
			node<Key, Value, Max_level, Alloc>* searched_key = search_key(key);
			if (searched_key == tail)
			{
				throw std::out_of_range("Out of range!");
			}
			auto& value = searched_key->get_value();
			return value;
		}

		Value& at(const Key& key)
		{
			return const_cast<Value&>(const_cast<const skip_list*>(this)->at(key));
		}

		template<class Pair>
			requires std::is_convertible_v<std::pair<Key, Value>, Pair>
		std::pair<iterator, bool> insert(Pair&& value_nods)
		{
			Level<Max_level> level = random_tools::random_level(Max_level, random_number_generator);
			if (head == nullptr && tail == nullptr)
			{
				init_head_and_tail();
			}
			auto updated_nods = search_key_storing_past_elements(value_nods.first, level);
			auto found_element = updated_nods[0]->next();
			if (found_element != tail)
			{
				if (equal_key<Key>(found_element->get_key(), value_nods.first))
				{
					return std::pair<iterator, bool>(iterator(head, tail, found_element), false);
				}
			}
			auto new_node = new node<Key, Value, Max_level, Alloc>(std::forward<Pair>(value_nods), level);
			for (Level<Max_level> index = 0; index < level; ++index)
			{
				new_node->link_with_left_node(updated_nods[index], index);
				updated_nods[index]->get_right_node(index)->link_with_left_node(new_node, index);
				
			}
			if (level > list_lvl){list_lvl = level;}
			++list_size;
			return std::pair<iterator, bool>(iterator(head, tail, new_node), true);
		}

		void erase(iterator position)
		{
			if (position.get_node_value() == tail)
			{
				return;
			}
			delete_node(position.get_node_value());
		}

		size_type erase(const Key& key)
		{
			auto del_element = search_key(key);
			if (del_element != tail)
			{
				delete_node(del_element);
			}
			return list_size;
		}

		void erase(iterator first, iterator last)
		{
			if (first == last)
			{
				return;
			}
			for (auto iterator_ = first; iterator_ != last;)
			{
				erase(iterator_++);
			}
		}

		void swap(skip_list& another) noexcept
		{
			if(this == &another)
			{
				return;
			}
			std::swap(list_lvl, another.list_lvl);
			std::swap(list_size, another.list_size);
			std::swap(allocator, another.allocator);
			std::swap(compare, another.compare);
			std::swap(head, another.head);
			std::swap(tail, another.tail);
		}

		void clear()
		{
			erase(begin(), end());
		}

		[[nodiscard]] const_iterator find(const Key& key) const
		{
			auto searched_node = search_key(key);
			if (searched_node == tail)
			{
				return const_iterator(head, tail, tail);
			}
			if (equal_key(searched_node->get_key(), key))
			{
				return const_iterator(head, tail, searched_node);
			}
			return const_iterator(head, tail, tail);
		}

		iterator find(const Key& key)
		{
			auto searched_node = search_key(key);
			if (searched_node == tail)
			{
				return iterator(head, tail, tail);
			}
			if (equal_key(searched_node->get_key(), key))
			{
				return iterator(head, tail, searched_node);
			}
			return iterator(head, tail, tail);
		}

		reverse_iterator rbegin() { return reverse_iterator(iterator(head, tail, tail)); }
		reverse_iterator rend() { return reverse_iterator(iterator(head, tail, head->next())); }
		[[nodiscard]] const_reverse_iterator rbegin() const { return const_reverse_iterator(const_iterator(head, tail, tail)); }
		[[nodiscard]] const_reverse_iterator rend() const { return const_reverse_iterator(const_iterator(head, tail, head->next())); }

		[[nodiscard]] size_type count(const Key& key) const
		{
			auto searched_node = search_key(key);
			if (searched_node == tail)
			{
				return 0;
			}
			return 1;
		}

		template <size_t Other_Max_level>
			requires comparable_value<Value>
		bool operator==(const skip_list<Key, Value, Compare, Other_Max_level, Alloc>& another) const
		{
			if (another.size() != list_size || empty() || another.empty())
			{
				return false;
			}
			auto another_node = another.cbegin();
			auto node = cbegin();
			for (size_t index = 0; index != list_size; ++index)
			{
				if (!equal_key((*node).first, (*another_node).first) ||
					(*node).second != (*another_node).second)
				{
					return false;
				}
				++node;
				++another_node;
			}
			return true;
		}

		template <size_t Other_Max_level>
			requires comparable_value<Value>
		bool operator!=(const skip_list<Key, Value, Compare, Other_Max_level, Alloc>& another) const
		{
			return !(*this == another);
		}
	};
}