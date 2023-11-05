#pragma once
#include <memory>
#include <utility>
#include <vector>
#include <memory>
#include <iterator>
#include <concepts>
#include <type_traits>
#include "random_number.h"
#include "skip_list_exception.h"



namespace skip_list_space
{
	constexpr size_t Max_Level = 10;

	template <class Compare, class value_type >
	concept is_compare = std::is_copy_constructible_v<Compare> && requires(Compare predicate,
		const Compare const_predicate, const  value_type value_first, const value_type value_second)
	{
		{predicate.operator()(value_first, value_second)} -> std::same_as<bool>;
		{const_predicate.operator()(value_first, value_second)} -> std::same_as<bool>;
	};

	template <class Value>
	concept Valid_Value = std::is_copy_constructible_v<Value>;

	template <class Key>
	concept Valid_Key = std::is_copy_constructible_v<Key>;

	template <typename Key,
		typename Value, typename Alloc = std::allocator<std::pair<const Key, Value>>>
	class node final
	{
		using pointer_list = std::vector<node<Key, Value, Alloc>*>;
		using reference = std::pair<const Key, Value>&;
		pointer_list pointer_list_right{};
		pointer_list pointer_list_left{};
		std::pair<const Key, Value>* node_value = nullptr;
		size_t level{};
		Alloc allocator{};
	public:
		node() = default;
		node(const std::pair<const Key, Value>& node_value_, const size_t level_, const Alloc& alloc = Alloc())
			: level(level_), allocator(alloc)
		{
			node_value = std::allocator_traits<Alloc>::allocate(allocator, 1);
			std::allocator_traits<Alloc>::construct(allocator, node_value, node_value_.first, std::move_if_noexcept(node_value_.second));
			pointer_list_left.assign(level_, nullptr);
			pointer_list_right.assign(level_, nullptr);
		}

		node(const node& other) noexcept//other node not have reference on this node 
		{
			allocator = other.allocator;
			pointer_list_right = other.pointer_list_right;
			pointer_list_left = other.pointer_list_left;
			level = other.level;
			node_value = std::allocator_traits<Alloc>::allocate(allocator, 1);
			std::allocator_traits<Alloc>::construct(allocator, node_value, other.node_value->first, other.node_value->first);
		}

		node(const node&& other) noexcept
		{
			allocator = std::move(other.allocator);
			pointer_list_right = std::move(other.pointer_list_right);
			pointer_list_left = std::move(other.pointer_list_left);
			level = std::move(other.level);
			node_value = other.node_value;
			other.node_value = nullptr;
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
			*this = node(other);
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

		void set_right_reference(const size_t index, node* value_) noexcept
		{
			pointer_list_right[index] = value_;
		}

		void set_left_reference(const size_t index, node* value_) noexcept
		{
			pointer_list_left[index] = value_;
		}

		static void bind_node(node* first, node* second, size_t level)
		{
			first->level = level;
			second->level = level;
			second->pointer_list_right.assign(level, nullptr);
			first->pointer_list_left.assign(level, nullptr);
			second->pointer_list_left.assign(level, first);
			first->pointer_list_right.assign(level, second);
		}

		void set_value(const Value& value_) { node_value->second = value_; }
		decltype(auto) get_element() { return *node_value; }

		node* get_right_reference(const size_t index) noexcept { return pointer_list_right[index]; }
		node* get_left_reference(const size_t index) noexcept { return pointer_list_left[index]; }
		node* next() { return pointer_list_right[0]; }
		node* prev() { return pointer_list_left[0]; }

		[[nodiscard]] size_t get_level() const { return level; }
		[[nodiscard]] const Key& get_key() { return node_value->first; }
		[[nodiscard]] const Key& get_key() const { return node_value->first; }
		[[nodiscard]] Value& get_value() { return node_value->second; }
		[[nodiscard]] const Value& get_value() const { return node_value->second; }
	};

	template <bool IsConst, typename Key, typename Value, typename Alloc >
	class node_iterator final
	{
		node<Key, Value, Alloc>* list_begin;
		node<Key, Value, Alloc>* list_end;
		node<Key, Value, Alloc>* node_pointer;

		void out_of_range_check(node<Key, Value, Alloc>* boundary) const
		{
			if (node_pointer == boundary)
			{
				throw std::out_of_range("out of range");
			}
		}

	public:
		using value_type = std::pair<const Key, Value>;
		using reference = std::pair<const Key, Value>&;
		using pointer = std::pair<const Key, Value>*;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::bidirectional_iterator_tag;
		using condition_ref = std::conditional_t<IsConst, std::add_const_t<std::remove_reference_t<reference>>&, reference>;
		node_iterator(node<Key, Value, Alloc>* begin_, node<Key, Value, Alloc>* end_, node<Key, Value, Alloc>* node_ptr)
			:list_begin(begin_), list_end(end_), node_pointer(node_ptr) {}

		condition_ref operator*()
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

	template <Valid_Key Key,
		Valid_Value Value,
		typename Compare = std::less<Key>,
		typename Alloc = std::allocator<std::pair<const Key, Value>>>
		requires is_compare<Compare, Key>
	class skip_list final {

		node<Key, Value, Alloc>* head;
		node<Key, Value, Alloc>* tail;
		Compare compare;
		Alloc allocator;
		size_t list_size{};
		size_t list_lvl{};

		[[nodiscard]] bool equal_key(const Key& first, const Key& second) const
		{
			return !compare(first, second) && !compare(second, first);
		}

		decltype(auto) search_key_storing_past_elements(Key key, const size_t level)
		{
			std::vector<node<Key, Value>*> past_elements;
			past_elements.assign(Max_Level, nullptr);
			auto node = head;
			for (int lvl_index = static_cast<int>(level) - 1; lvl_index >= 0; --lvl_index)
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

		[[nodiscard]] node<Key, Value, Alloc>* search_key(Key key) const
		{
			auto node = head;
			for (int lvl_index = static_cast<int>(list_lvl) - 1; lvl_index >= 0; --lvl_index)
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

		size_t find_max_lvl()
		{
			size_t max = 0;
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

		void delete_node(node<Key, Value>* del_node)
		{
			if (del_node != head & del_node != tail)
			{
				auto lvl = del_node->get_level();
				for (size_t lvl_index = 0; lvl_index < lvl; ++lvl_index)
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
		using iterator = node_iterator<false, Key, Value, Alloc>;
		using const_iterator = node_iterator<true, Key, Value, Alloc>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using value_type = std::pair<const Key, Value>;
		using size_type = size_t;

		explicit skip_list(const Compare& comp = Compare(), const Alloc& alloc = Alloc()) : compare(comp), allocator(alloc)
		{
			head = new node<Key, Value, Alloc>;
			tail = new node<Key, Value, Alloc>;
			list_lvl = 0;
			node<Key, Value>::bind_node(head, tail, Max_Level);
		}

		~skip_list()
		{
			delete_list();
		}

		skip_list(const skip_list& another) noexcept : compare(another.compare), allocator(another.allocator)
		{

			list_size = 0;
			list_lvl = 0;
			head = new node<Key, Value, Alloc>;
			tail = new node<Key, Value, Alloc>;
			node<Key, Value>::bind_node(head, tail, Max_Level);
			for (const_iterator node = another.cbegin(); node != another.cend(); ++node)
			{
				this->insert(std::make_pair((*node).first, (*node).second));
			}
		}

		skip_list(skip_list&& another) noexcept : compare(std::move(another.compare)), allocator(std::move(another.allocator)),
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
			list_size = another.list_size;
			list_lvl = another.list_lvl;
			head = new node<Key, Value>;
			tail = new node<Key, Value>;
			for (auto list_node : another)
			{
				insert(std::make_pair((*list_node).get_key(), (*list_node).get_value()));
			}
			return *this;
		}

		skip_list& operator=(skip_list&& another) noexcept
		{
			if (this == &another)
			{
				return *this;
			}
			compare = std::move(another.compare);
			allocator = std::move(another.allocator);
			list_size = another.list_size;
			list_lvl = another.list_lvl;
			std::swap(head, another.head);
			std::swap(tail, another.tail);
			another.list_lvl = 0;
			another.list_size = 0;
			return *this;
		}

		iterator begin() { return iterator(head, tail, head->next()); }

		iterator end() { return iterator(head, tail, tail); }

		[[nodiscard]] const_iterator cbegin() const { return const_iterator(head, tail, head->next()); }

		[[nodiscard]] const_iterator cend() const { return const_iterator(head, tail, tail); }

		[[nodiscard]] bool empty() const { return list_size == 0; }

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
			node<Key, Value, Alloc>* searched_key = search_key(key);
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
			auto [key, value] = value_nods;
			size_t level = random_level(Max_Level);
			node<Key, Value, Alloc>* new_node = nullptr;
			if (list_size)
			{
				auto updated_nods = search_key_storing_past_elements(key, level);
				auto found_element = updated_nods[0]->next();
				if (found_element != tail)
				{
					if (equal_key(found_element->get_key(), key))//key == found_element.key
					{
						found_element->set_value(value);
						return std::pair<iterator, bool>(iterator(head, tail, found_element), false);
					}
				}
				new_node = new node<Key, Value>(value_nods, level);
				for (size_t index = 0; index < level; ++index)
				{
					auto updated_node = updated_nods[index];
					new_node->set_right_reference(index, updated_node->get_right_reference(index));
					updated_node->get_right_reference(index)->set_left_reference(index, new_node);
					new_node->set_left_reference(index, updated_node);
					updated_node->set_right_reference(index, new_node);
				}
			}
			else
			{
				new_node = new node<Key, Value>(value_nods, level);
				for (size_t index = 0; index < level; ++index)
				{
					head->set_right_reference(index, new_node);
					tail->set_left_reference(index, new_node);
					new_node->set_left_reference(index, head);
					new_node->set_right_reference(index, tail);
				}
			}
			if (level > list_lvl)
			{
				list_lvl = level;
			}
			list_size++;
			return std::pair<iterator, bool>(iterator(head, tail, new_node), true);
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

		bool operator==(const skip_list& another) const
		{
			if (another.size() != list_size)
			{
				return false;
			}
			auto another_node = another.cbegin();
			auto node = cbegin();
			for (size_t index = 0; index != list_size; ++index)
			{
				if (compare((*node).first, (*another_node).first) || compare((*another_node).first, (*node).first))
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

		bool operator!=(const skip_list& another) const
		{
			return !(another == *this);
		}
	};
}
