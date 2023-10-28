#pragma once
#include <memory>
#include <utility>
#include <vector>
#include "random_number.h"
#include "skip_list_exception.h"

constexpr size_t Max_Level = 10;

template <typename Key,
	typename Value>
class node final
{
	using pointer_list = std::vector<std::shared_ptr<node<Key, Value>>>;
	pointer_list pointer_list_right{};
	pointer_list pointer_list_left{};
	Key key{};
	Value value{};
	size_t level{};
public:
	node() = default;
	node(Key key_, Value val_, size_t level_) :key(key_), value(val_), level(level_)
	{
		pointer_list_left.assign(level_, nullptr);
		pointer_list_right.assign(level_, nullptr);
	}

	void set_value(const Value& value_) { value = value_; }

	void set_right_reference(size_t index, const std::shared_ptr<node<Key, Value>>& value) noexcept
	{

		pointer_list_right[index] = value;

	}

	void set_left_reference(size_t index, const std::shared_ptr<node<Key, Value>>& value) noexcept {
		pointer_list_left[index] = value;
	}

	decltype(auto) get_right_reference(size_t index) noexcept { return pointer_list_right[index]; }

	decltype(auto) get_left_reference(size_t index) noexcept { return pointer_list_left[index]; }

	pointer_list& get_list_left_nods() { return pointer_list_left; }

	pointer_list& get_list_right_nods() { return pointer_list_right; }

	size_t get_level() const { return level; }

	Key get_key() { return key; }
	Value get_value() { return value; }

};

template <typename Key, typename Value>
class const_node_iterator
{
protected:
	std::shared_ptr<node<Key, Value>> list_begin;
	std::shared_ptr<node<Key, Value>> list_end;
	std::shared_ptr<node<Key, Value>> node_pointer;

	void boundary_check()
	{
		if (node_pointer == list_end || node_pointer == list_begin)
		{
			throw std::out_of_range("out of range");
		}
	}

public:
	using value_type = node<Key, Value>;
	using reference = node<Key, Value>&;
	using pointer = std::shared_ptr<node<Key, Value>>;
	/*using difference_type = typename std::iterator_traits<std::shared_ptr<node<Key, Value>>>::difference_type;*/
	using iterator_category = std::bidirectional_iterator_tag;

	const_node_iterator(pointer list_begin_, pointer list_end_) :
		list_begin(list_begin_), list_end(list_end_), node_pointer(list_begin_) {}

	reference operator*() const
	{
		if (node_pointer == list_end)
		{
			throw error_dereferencing_end();
		}
		return *node_pointer;
	}

	const_node_iterator& operator++()
	{
		boundary_check();
		node_pointer = node_pointer->get_right_reference(0);
		return *this;
	}

	const_node_iterator& operator--()
	{
		boundary_check();
		node_pointer = node_pointer->get_left_reference(0);
		return *this;
	}

	bool operator==(const pointer& other) const noexcept
	{
		return node_pointer == other.node_pinter;
	}
};

template <typename Key, typename Value>
class node_iterator : public const_node_iterator<Key, Value>
{
public:
	using value_type = node<Key, Value>;
	using reference = node<Key, Value>&;
	using pointer = std::shared_ptr<node<Key, Value>>;
	/*using difference_type = typename std::iterator_traits<std::shared_ptr<node<Key, Value>>>::difference_type;*/
	using iterator_category = std::bidirectional_iterator_tag;

	reference operator*()
	{
		if (this->node_pointer == this->list_end)
		{
			throw error_dereferencing_end();
		}
		return *(this->node_pointer);
	}
};

template <typename Key,
	typename Value,
	typename Compare = std::less<Key>,
	typename Alloc = std::allocator<std::pair<const Key, Value> >>

	class skip_list final {

	std::shared_ptr <node<Key, Value>> head;
	std::shared_ptr <node<Key, Value>> tail;
	size_t list_size{};
	Compare compare;
	Alloc allocator;
	size_t list_lvl;

	decltype(auto) search_key_storing_past_elements(Key key, size_t level)
	{
		std::vector<std::shared_ptr<node<Key, Value>>> past_elements;
		past_elements.assign(Max_Level, nullptr);
		auto nods = head;
		for (int lvl_index = static_cast<int>(level) - 1; lvl_index >= 0; --lvl_index)
		{
			while (nods->get_right_reference(lvl_index) != tail &&
				compare(nods->get_right_reference(lvl_index)->get_key(), key))
			{
				nods = nods->get_right_reference(lvl_index);
			}
			past_elements[lvl_index] = nods;
		}
		return past_elements;
	}

	decltype(auto) search_key(Key key)
	{
		auto nods = head->get_list_right_nods();
		for (size_t lvl_index = list_lvl - 1; lvl_index != 0; --lvl_index)
		{
			while (nods[lvl_index] != nullptr &&
				compare(nods[lvl_index]->get_key(), key))
			{
				nods = nods[lvl_index]->get_list_right_nods();
			}
		}
		return nods[0];
	}

	void delete_node(const Key& searched_key)
	{
		auto updated_nods = search_key_storing_past_elements(searched_key);
		auto element = updated_nods[0];
		if (element->get_key() == searched_key)
		{
			auto lvl = element->get_level();
			for (int level_index = 0; level_index < lvl; ++level_index)
			{
				updated_nods[level_index]->get_list_right_nods();
			}
		}
		/*Delete(list, searchKey)
			local update[1..MaxLevel]
			x : = list→header
			for i : = list→level downto 1 do
				while x→forward[i]→key < searchKey do
					x : = x→forward[i]
			update[i] : = x
		x : = x→forward[1]
		if x→key = searchKey then
			for i : = 1 to list→level do
				if update[i]→forward[i] ≠ x then break
				update[i]→forward[i] : = x→forward[i]
			free(x)
			while list→level > 1 and list→header→forward[list→level] = NIL do
				list→level : = list→level – 1*/
	}
	public:

		using iterator = node_iterator<Key, Value>;
		using const_iterator = const node_iterator<Key, Value>;
		using value_type = std::pair<const Key, Value>;

		explicit skip_list(const Compare& comp, const Alloc& alloc = Alloc()) : compare(comp), allocator(alloc)
		{
			head = std::make_shared<node<Key, Value>>();
			tail = std::make_shared<node<Key, Value>>();
			list_lvl = 0;
			tail->get_list_right_nods().assign(Max_Level, nullptr);
			head->get_list_left_nods().assign(Max_Level, nullptr);
			head->get_list_right_nods().assign(Max_Level, tail);
			tail->get_list_left_nods().assign(Max_Level, head);
		}
		skip_list(const skip_list& another) {}
		skip_list& operator=(const skip_list& another);

		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;

		[[nodiscard]] bool empty() const
		{
			return list_size == 0;
		}

		size_t size() const
		{
			return list_size;
		}

		/*Value& operator[](const Key& key);
		Value& at(const Key& key);
		const Value& at(const Key& key);*/

		/*std::pair<iterator, bool>*/bool insert(const value_type& parameters)
		{
			size_t level = random_level(Max_Level);
			std::shared_ptr <node<Key, Value>> new_node;
			if (list_size)
			{
				auto updated_nods_pointer = search_key_storing_past_elements(parameters.first, level);
				auto element = updated_nods_pointer[0];
				if (element->get_key() == parameters.first)
				{
					element->set_value(parameters.second);
					return /*std::pair<iterator, bool>(node_iterator<Key, Value>(head, tail), false)*/false;
				}

				new_node = std::make_shared<node<Key, Value>>(parameters.first, parameters.second, level);				
				const size_t update_nods_size = updated_nods_pointer.size() - 1;
				for (size_t index = 0; index < level; ++index)
				{
					auto updated_node = std::move(updated_nods_pointer[index]);
					new_node->set_right_reference(index, updated_node->get_right_reference(index));
					updated_node->get_right_reference(index)->set_left_reference(index, new_node);
					new_node->set_left_reference(index, updated_node);
					updated_node->set_right_reference(index, new_node);
				}
			}
			else
			{
				new_node = std::allocate_shared<node<Key, Value>>(allocator, parameters.first, parameters.second, level);
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
			return /*std::pair<iterator, bool>(node_iterator<Key, Value>(new_node, tail), true)*/true;
		}

		/*void erase(iterator position);
		size_type erase(const Key& key);
		void erase(iterator first, iterator last);

		void swap(skip_list& another);
		void clear();

		iterator find(const Key& key);
		const_iterator find(const Key& key) const;
		typedef .... reverse_iterator;
		typedef .... const_reverse_iterator;

		reverse_iterator rbegin();
		reverse_iterator rend();
		const_reverse_iterator rbegin() const;
		const_reverse_iterator rend() const;

		size_t count(const Key &key) const;*/
};

template <typename K, typename V, typename C, typename A>
inline bool operator==(const skip_list<K, V, C, A>& x, const skip_list<K, V, C, A>& y) {
	// ....
}

template <typename K, typename V, typename C, typename A>
inline bool operator!=(const skip_list<K, V, C, A>& x, const skip_list<K, V, C, A>& y) {
	// ....
}