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
		void max_boarder_check(size_t other_size) const
		{
			if (size_ + other_size > Max_level)
			{
				throw std::out_of_range("level is larger than max");
			}
		}

		void min_boarder_check(size_t other_size) const
		{
			if (size_ < other_size)
			{
				throw std::out_of_range("level is less than minimum");
			}
		}
	public:
		
		Level() = default;
		constexpr Level(const size_t size)
		{
			max_boarder_check(size);
			size_ = size;
		}

		operator size_t() const { return size_; }
		size_t get_size() const { return size_; }
		void set_size(size_t size) { size_ = size; }

		auto operator<=>(const Level& other) const = default;
		bool operator<(const size_t& other) const { return size_ < other; }
		bool operator>(const size_t& other) const { return size_ > other; }
		bool operator<=(const size_t& other) const { return size_ <= other; }
		bool operator>=(const size_t& other) const { return size_ >= other; }
		bool operator==(const size_t& other_size) const noexcept{ return size_ == other_size; }

		template<typename Size_type>
		requires std::is_same_v<Size_type, size_t>
		Level& operator=(Size_type&& other_size)
		{
			min_boarder_check(other_size);
			max_boarder_check(other_size);
			size_ = other_size;
			return *this;
		}

		Level& operator-=(const size_t& other_size) 
		{
			min_boarder_check(other_size);
			size_ -= other_size;
			return *this;
		}

		Level& operator+=(const size_t& other_size) 
		{
			max_boarder_check(other_size);
			size_ += other_size;
			return *this;
		}

		[[nodiscard]] Level operator-(const size_t& other_size) const 
		{
			min_boarder_check(other_size);
			return Level(size_ - other_size);
		}

		[[nodiscard]] Level operator+(const size_t& other_size) const 
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
		using pointer_list = std::vector<node<Key, Value, Max_level, Alloc>*>;
		using reference = std::pair<const Key, Value>&;
		pointer_list pointer_list_right{};//норм название
		pointer_list pointer_list_left{};
		std::pair<const Key, Value>* node_value = nullptr;
		Level<Max_level> level{};
		Alloc allocator{};
		bool valid = false;
	public:
		node() = default;
		node(const std::pair<const Key, Value>& node_value_, const Level<Max_level> level_, const Alloc& alloc = Alloc())
			: level(level_), allocator(alloc)
		{
			node_value = std::allocator_traits<Alloc>::allocate(allocator, 1);
			std::allocator_traits<Alloc>::construct(allocator, node_value, node_value_.first, node_value_.second);
			pointer_list_left.assign(level_, nullptr);
			pointer_list_right.assign(level_, nullptr);
			valid = true;
		}

		node(const node& other) noexcept
			: level(other.level), allocator(other.allocator)
		{
			pointer_list_left.assign(other.level, nullptr);
			pointer_list_right.assign(other.level, nullptr);
			node_value = std::allocator_traits<Alloc>::allocate(allocator, 1);
			std::allocator_traits<Alloc>::construct(allocator, node_value, other.node_value->first, other.node_value->second);
			valid = true;
		}

		node(node&& other) noexcept
		{
			allocator = std::move_if_noexcept(other.allocator);
			pointer_list_right = std::move_if_noexcept(other.pointer_list_right);
			pointer_list_left = std::move_if_noexcept(other.pointer_list_left);
			level = std::move_if_noexcept(other.level);
			node_value = other.node_value;
			other.node_value = nullptr;
			other.valid = false;
			valid = true;
		}

		node& operator=(const node& other)
		{
			if (this == &other)
			{
				return *this;
			}
			allocator = other.allocator;
			pointer_list_right = other.pointer_list_right;
			pointer_list_left = other.pointer_list_left;
			level = other.level;
			if (node_value != nullptr)
			{
				allocator.deallocate(node_value, sizeof(*node_value));
			}
			node_value = allocator.allocate(sizeof(*other.node_value));
			node_value->first = other.node_value->first;
			node_value->second = other.node_value->second;
			return *this;
		}

		node& operator=(node&& other) noexcept
		{
			if (this == &other)
			{
				return *this;
			}
			*this = node(std::move_if_noexcept(other));
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

		void set_right_reference(const size_t index, node* value_) 
		{
			if(this == value_){	return;	}
			pointer_list_right.at(index) = value_;
		}

		void set_left_reference(const size_t index, node* value_) 
		{
			if (this == value_) { return; }
			pointer_list_left.at(index) = value_;
		}

		[[nodiscard]] bool is_valid() const noexcept
		{
			return valid;
		}

		static void bind_node(node* first, node* second, Level<Max_level> level_) noexcept
		{
			auto level = level_.get_size();
			first->level = level;
			second->level = level;
			second->pointer_list_right.assign(level, nullptr);
			second->pointer_list_left.assign(level, first);
			first->pointer_list_left.assign(level, nullptr);
			first->pointer_list_right.assign(level, second);
		}

		void set_value(const Value& value_) { node_value->second = value_; }
		decltype(auto) get_element() { return *node_value; }
		[[nodiscard]] decltype(auto) get_element() const  { return *node_value; }
		
		node* get_right_reference(const size_t  index) noexcept { return pointer_list_right[index]; }
		node* get_left_reference(const size_t index) noexcept { return pointer_list_left[index]; }
		node* next() { return pointer_list_right.at(0); }
		[[nodiscard]] const node* next() const { return pointer_list_right.at(0); }
		node* prev() { return pointer_list_left.at(0); }
		[[nodiscard]] const node* prev() const{ return pointer_list_left.at(0); }

		bool operator==(const node& other) const
		{
			return node_value->first == other.node_value->first && node_value->second == other.node_value->second;
		}

		[[nodiscard]] Level<Max_level> get_level() const { return level; }
		[[nodiscard]] const Key& get_key() { return node_value->first; }
		[[nodiscard]] const Key& get_key() const { return node_value->first; }
		[[nodiscard]] Value& get_value() { return node_value->second; }
		[[nodiscard]] const Value& get_value() const { return node_value->second; }
		[[nodiscard]] const pointer_list& get_right_reference_list() const { return pointer_list_right; }
		[[nodiscard]] const pointer_list& get_left_reference_list() const { return pointer_list_left; }
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

		condition_ref operator*()//конструктор копирования от const
		{
			if (node_pointer == list_end)
			{
				throw error_dereferencing_end();
			}
			return node_pointer->get_element();
		}

		bool operator==(const node_iterator& other) const noexcept
		{
			return node_pointer == other.node_pointer;
		}

		bool operator!=(const node_iterator& other) const noexcept
		{
			return node_pointer != other.node_pointer;
		}

		decltype(auto) get_element()
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
		typename Alloc = std::allocator<std::pair<const Key, Value>>>
		requires is_compare<Compare, Key>
	class skip_list final {

		node<Key, Value, Max_level, Alloc>* head;
		node<Key, Value, Max_level, Alloc>* tail;
		Compare compare;
		Alloc allocator;
		size_t list_size{};
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

		void insert_nodes(node<Key, Value, Max_level, Alloc>* nodes_head, node<Key, Value, Max_level, Alloc>* nodes_tail)
		{
			std::vector<node<Key, Value, Max_level, Alloc>*> array_no_linked_nodes;
			array_no_linked_nodes.assign(Max_level, head);
			for (auto inserted_node = nodes_head->next(); inserted_node != nodes_tail; inserted_node = inserted_node->next())
			{
				auto new_node = new node<Key, Value, Max_level, Alloc>(*inserted_node);
				for (Level<Max_level> index = 0; index < new_node->get_level(); ++index)
				{
					new_node->set_left_reference(index, array_no_linked_nodes[index]);
					array_no_linked_nodes[index]->set_right_reference(index, new_node);
					array_no_linked_nodes[index] = new_node;
				}
			}
			for (size_t index = 0; index < Max_level; ++index)
			{
				tail->set_left_reference(index, array_no_linked_nodes[index]);
				array_no_linked_nodes[index]->set_right_reference(index, tail);
			}
		}

		decltype(auto) insert_node_into_nonempty_list(const std::pair<const Key, Value>& value_node, Level<Max_level> level)
		{
			auto updated_nods = search_key_storing_past_elements(value_node.first, level);
			auto found_element = updated_nods[0]->next();
			if (found_element != tail)
			{
				if (equal_key<Key>(found_element->get_key(), value_node.first))
				{
					found_element->set_value(value_node.second);
					return std::pair<iterator, bool>(iterator(head, tail, found_element), false);
				}
			}
			auto new_node = new node<Key, Value, Max_level, Alloc>(value_node, level);
			for (Level<Max_level> index = 0; index < level; ++index)
			{
				auto updated_node = updated_nods[index];
				new_node->set_right_reference(index, updated_node->get_right_reference(index));
				updated_node->get_right_reference(index)->set_left_reference(index, new_node);
				new_node->set_left_reference(index, updated_node);
				updated_node->set_right_reference(index, new_node);
			}
			if (level > list_lvl)
			{
				list_lvl = level;
			}
			++list_size;
			return std::pair<iterator, bool>(iterator(head, tail, new_node), true);
		}

		decltype(auto) search_key_storing_past_elements(Key key, const Level<Max_level> level)
		{
			std::vector<node<Key, Value, Max_level, Alloc>*> past_elements;
			past_elements.assign(Max_level, nullptr);
			auto node = head;
			for (int lvl_index = static_cast<int>(level.get_size()) - 1; lvl_index >= 0; --lvl_index)
			{
				while (node->get_right_reference(lvl_index) != tail &&
					compare(node->get_right_reference(lvl_index)->get_key(), key))
				{
					node = node->get_right_reference(lvl_index);
				}
				past_elements[lvl_index] = node;
			}
			return past_elements;
		}

		[[nodiscard]] node<Key, Value, Max_level, Alloc>* search_key(Key key) const
		{
			auto node = head;
			for (int lvl_index = static_cast<int>(list_lvl.get_size()) - 1; lvl_index >= 0; --lvl_index)
			{
				while (node->get_right_reference(lvl_index) != tail &&
					compare(node->get_right_reference(lvl_index)->get_key(), key))
				{
					node = node->get_right_reference(lvl_index);
				}
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
			for (const_iterator const_iter = cbegin(); const_iter != cend(); ++const_iter)
			{
				auto new_max = const_iter.get_element()->get_level();
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
					auto next_element = del_node->get_right_reference(lvl_index);
					auto prev_element = del_node->get_left_reference(lvl_index);
					next_element->set_left_reference(lvl_index, prev_element);
					prev_element->set_right_reference(lvl_index, next_element);
				}
				list_size -= 1;
			}
			list_lvl = find_max_lvl();
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

		

	public:
		using iterator = node_iterator<false, Key, Value, Max_level, Alloc>;
		using const_iterator = node_iterator<true, Key, Value, Max_level, Alloc>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using value_type = std::pair<const Key, Value>;
		using size_type = size_t;

		explicit skip_list(const Compare& comp = Compare(), const Alloc& alloc = Alloc()) : compare(comp), allocator(alloc)
		{
			head = new node<Key, Value, Max_level, Alloc>;
			tail = new node<Key, Value, Max_level, Alloc>;
			list_lvl = 0;
			node<Key, Value, Max_level, Alloc>::bind_node(head, tail, Max_level);
		}

		explicit skip_list(const std::initializer_list<value_type>& list, const Compare& comp = Compare(), const Alloc& alloc = Alloc())
		: skip_list(comp, alloc)
		{
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
			head = new node<Key, Value, Max_level, Alloc>;
			tail = new node<Key, Value, Max_level, Alloc>;
			node<Key, Value, Max_level, Alloc>::bind_node(head, tail, Max_level);
			if (!another.empty())
			{
				insert_nodes(another.head, another.tail);
			}
		}

		skip_list(skip_list&& another) noexcept : compare(std::move_if_noexcept(another.compare)),
			allocator(std::move_if_noexcept(another.allocator)),
				list_size(another.list_size), list_lvl(another.list_lvl)
		{
			head = another.head;
			tail = another.tail;
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
			head = new node<Key, Value, Max_level, Alloc>;
			tail = new node<Key, Value, Max_level, Alloc>;
			node<Key, Value, Max_level, Alloc>::bind_node(head, tail, Max_level);
			if (!another.empty())
			{
				insert_nodes(another.head, another.tail);
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

		iterator begin() { return iterator(head, tail, head->next()); }

		iterator end() { return iterator(head, tail, tail); }

		[[nodiscard]] const_iterator cbegin() const { return const_iterator(head, tail, head->next()); }

		[[nodiscard]] const_iterator cend() const { return const_iterator(head, tail, tail); }

		[[nodiscard]] bool empty() const { return list_size == static_cast<size_t>(0); }

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
			requires std::is_convertible_v<Pair, const std::pair<const Key, Value>>
		std::pair<iterator, bool> insert(Pair&& value_nods)
		{
			Level<Max_level> level = random_tools::random_level(Max_level);
			if (empty())
			{
				auto new_node = new node<Key, Value, Max_level, Alloc>(value_nods, level);
				for (Level<Max_level> index = 0; index < level; ++index)
				{
					head->set_right_reference(index, new_node);
					tail->set_left_reference(index, new_node);
					new_node->set_left_reference(index, head);
					new_node->set_right_reference(index, tail);
				}
				if (level > list_lvl.get_size())
				{
					list_lvl = level;
				}
				++list_size;
				return std::pair<iterator, bool>(iterator(head, tail, new_node), true);
			}
			return insert_node_into_nonempty_list(value_nods, level);
		}

		void erase(iterator position)
		{
			if (position.get_element() == tail)
			{
				return;
			}
			delete_node(position.get_element());
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
			for (auto iter = first; iter != last;)
			{
				erase(iter++);
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
				if (!equal_key((*node).first, (*another_node).first))
				{
					return false;
				}
				if ((*node).second != (*another_node).second)
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
			return !(another == *this);
		}
	};
}
