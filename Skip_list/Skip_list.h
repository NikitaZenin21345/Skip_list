#pragma once
#include <memory>
#include <utility>
#include <vector>
#include "random_number.h"

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
	size_t level;
public:
	node() = default;
	node(Key key_, Value val_, size_t level_) :key(key_), value(val_), level(level_)
	{
		pointer_list_left.assign(level_, nullptr);
		pointer_list_right.assign(level_, nullptr);
	}
	void set_value(const Value& value_)
	{
		value = value_;
	}
	node(const node& other)
	{

	}
	pointer_list& get_list_left_nods()
	{
		return pointer_list_left;
	}
	pointer_list& get_list_right_nods()
	{
		return pointer_list_right;
	}
	size_t get_level() const
	{
		return level;
	}
	Key get_key() { return key; }
	Value get_value() { return value; }

};

template <typename Key,
	typename Value>
class node_iterator final
{
	std::shared_ptr<node<Key, Value>> current_node;
public:
	using iterator_category = std::bidirectional_iterator_tag;
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
	public:

		using iterator = node_iterator<Key, Value>;
		using const_iterator = const node_iterator<Key, Value>;
		using value_type = std::pair<const Key, Value>;

		skip_list() = default;
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

		bool empty() const;
		size_t size() const;

		Value& operator[](const Key& key);
		Value& at(const Key& key);
		const Value& at(const Key& key);

		std::pair<iterator, bool> insert(const value_type& parameters)
		{
			size_t level = random_level(Max_Level);
			if (list_size)
			{
				std::vector<std::shared_ptr<node<Key, Value>>> updated_nods_pointer;
				updated_nods_pointer.assign(Max_Level, nullptr);
				auto nods = head->get_list_right_nods();
				for(size_t lvl_index = list_lvl; lvl_index != 0; --lvl_index)
				{
					while(nods[lvl_index]->get_list_right_nods()[lvl_index] != nullptr && compare(nods[lvl_index]->get_key(),parameters.first))
					{
						nods = nods[lvl_index]->get_list_right_nods();
					}
					updated_nods_pointer.push_back(nods[lvl_index]);
				}
				auto element = nods[0];
				if(element->get_key() == parameters.first)
				{
					element->set_value(parameters.second);
				}
				else
				{
					if(level > list_lvl)
					{
						for (auto index = list_lvl; index != level; ++index)
						{
							updated_nods_pointer[index] = head;
						}
						list_lvl = level;
							
					}
					auto new_node = std::make_shared<node>(parameters.first, parameters.second, level);
					auto new_node_list_right_nods = new_node->get_list_right_nods();
					auto new_node_list_left_nods = new_node->get_list_left_nods();
					for (int index = 0; index < level; ++index)
					{
						auto next_nodes = updated_nods_pointer[index]->get_list_right_nods()[index];
						new_node_list_right_nods[index] = next_nodes;
						new_node_list_left_nods[index] = updated_nods_pointer[index];
						next_nodes = new_node;
						next_nodes->get_list_left_nods() = new_node;
					}
				}
				/*local update[1..MaxLevel]
				x : = list→header
				for i : = list→level downto 1 do
					while x→forward[i]→key < searchKey do
						x : = x→forward[i]
					# x→key < searchKey ≤ x→forward[i]→key
					update[i] : = x
				x : = x→forward[1]
				if x→key = searchKey then x→value : = newValue
				else
					lvl : = randomLevel()
					if lvl > list→level then
						for i : = list→level + 1 to lvl do
							update[i] : = list→header
							list→level : = lvl
					x : = makeNode(lvl, searchKey, value)
				for i : = 1 to level do
					x→forward[i] : = update[i]→forward[i]
					update[i]→forward[i] : = x*/
			}
			else
			{
				auto head_list_right_nods = head->get_list_right_nods();
				auto tail_list_left_nods = tail->get_list_left_nods();
				auto new_node = std::make_shared<node>(parameters.first, parameters.second, level);
				for (size_t index = 0; index < level; ++index)
				{
					head_list_right_nods[index] = new_node;
				}
			}
			if(level > list_lvl)
			{
				list_lvl = level;
			}
			list_size++;
		}

		void erase(iterator position);
		size_type erase(const Key& key);
		void erase(iterator first, iterator last);

		void swap(skip_list& another);
		void clear();

		iterator find(const Key& key);
		const_iterator find(const Key& key) const;
};

template <typename K, typename V, typename C, typename A>
inline bool operator==(const skip_list<K, V, C, A>& x, const skip_list<K, V, C, A>& y) {
	// ....
}

template <typename K, typename V, typename C, typename A>
inline bool operator!=(const skip_list<K, V, C, A>& x, const skip_list<K, V, C, A>& y) {
	// ....
}