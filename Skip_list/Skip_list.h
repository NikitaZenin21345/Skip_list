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
	using pointer_list = std::vector<node<Key, Value>*>;
	using pointer = node<Key, Value>*;
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
	decltype(auto) get_element()
	{
		return std::make_pair(key, value);
	}

	void set_right_reference(const size_t index, node* value_) noexcept
	{
		pointer_list_right[index] = value_;
	}

	node* next() { return pointer_list_right[0]; }

	void set_left_reference(const size_t index, node* value_) noexcept {
		pointer_list_left[index] = value_;
	}

	node* get_right_reference(const size_t index) noexcept { return pointer_list_right[index]; }

	node* get_left_reference(const size_t index) noexcept { return pointer_list_left[index]; }

	static void bind_node(node* first, node* second, size_t level)
	{
		first->level = level;
		second->level = level;
		second->pointer_list_right.assign(level, nullptr);
		first->pointer_list_left.assign(level, nullptr);
		second->pointer_list_left.assign(level, first);
		first->pointer_list_right.assign(level, second);
	}

	[[nodiscard]] size_t get_level() const { return level; }

	[[nodiscard]] Key& get_key() { return key; }
	[[nodiscard]] const Key& get_key() const { return key; }
	[[nodiscard]] Value& get_value() { return value; }
	[[nodiscard]] const Value& get_value() const { return value; }

};

template <typename Key, typename Value>
class const_node_iterator
{
protected:
	node<Key, Value>* list_begin;
	node<Key, Value>* list_end;
	node<Key, Value>* node_pointer;

public:
	using value_type = Value;
	using reference = node<Key, Value>&;
	using pointer = node<Key, Value>*;
	using difference_type = typename std::pointer_traits<node<Key, Value>*>::difference_type;
	using iterator_category = std::bidirectional_iterator_tag;
	const_node_iterator(const pointer& begin_, const pointer& end_, const pointer& node_ptr) :list_begin(begin_), list_end(end_), node_pointer(node_ptr) {}
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
		if (node_pointer == list_end)
		{
			throw std::out_of_range("out of range");
		}
		node_pointer = node_pointer->next();
		return *this;
	}

	/*const_node_iterator& operator++(int)
	{
		if (node_pointer == list_end)
		{
			throw std::out_of_range("out of range");
		}
		node_pointer = node_pointer->get_right_reference(0);
		auto next_node = node_pointer;
		return next_node;
	}*/

	const_node_iterator& operator--()
	{
		if (node_pointer == list_begin)
		{
			throw std::out_of_range("out of range");
		}
		node_pointer = node_pointer->get_left_reference(0);
		return *this;
	}

	bool operator==(const const_node_iterator& other) const noexcept
	{
		return node_pointer == other.node_pointer;
	}

	bool operator!=(const const_node_iterator& other) const noexcept 
	{
		return node_pointer != other.node_pointer;
	}
};

template <typename Key, typename Value>
class node_iterator final : public const_node_iterator<Key, Value>
{
public:
	using value_type = node<Key, Value>;
	using reference = node<Key, Value>&;
	using pointer = node<Key, Value>*;
	using difference_type = typename std::pointer_traits<node<Key, Value>*>::difference_type;
	using iterator_category = std::bidirectional_iterator_tag;

	node_iterator(const pointer& begin_, const pointer& end_, const pointer& node_ptr)//naming
		:const_node_iterator<Key, Value>(begin_, end_, node_ptr) {}
	reference operator*()
	{
		if (this->node_pointer == this->list_end)
		{
			throw error_dereferencing_end();
		}
		return *(this->node_pointer);
	}
};

template <typename Key, typename Value>
class reverse_const_node_iterator : public const_node_iterator<Key, Value>
{
public:
	using value_type = node<Key, Value>;
	using reference = node<Key, Value>&;
	using pointer = node<Key, Value>*;
	using difference_type = typename std::pointer_traits<node<Key, Value>*>::difference_type;
	using iterator_category = std::bidirectional_iterator_tag;

	reverse_const_node_iterator(const pointer& begin_, const pointer& end_, const pointer& node_ptr)//naming
		:const_node_iterator<Key, Value>(begin_, end_, node_ptr) {}

	reverse_const_node_iterator& operator++() 
	{
		if (this->node_pointer == this->list_end)
		{
			throw std::out_of_range("out of range");
		}
		this->node_pointer = this->node_pointer->get_left_reference(0);
		return *this;
	}

	reverse_const_node_iterator& operator--()
	{
		if (this->node_pointer == this->list_end)
		{
			throw std::out_of_range("out of range");
		}
		this->node_pointer = this->node_pointer->next();
		return *this;
	}
};

template <typename Key, typename Value>
class reverse_node_iterator : public reverse_const_node_iterator<Key, Value>
{
public:
	using value_type = node<Key, Value>;
	using reference = node<Key, Value>&;
	using pointer = node<Key, Value>*;
	using difference_type = typename std::pointer_traits<node<Key, Value>*>::difference_type;
	using iterator_category = std::bidirectional_iterator_tag;

	reverse_node_iterator(const pointer& begin_, const pointer& end_, const pointer& node_ptr)//naming
		:reverse_const_node_iterator<Key, Value>(begin_, end_, node_ptr) {}

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
	typename Alloc = std::allocator<std::pair<const Key, Value>>>

class skip_list final {

	node<Key, Value>* head;
	node<Key, Value>* tail;
	Compare compare;
	Alloc allocator;
	size_t list_size{};
	size_t list_lvl;

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

	node<Key, Value>* search_key(Key key)
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
		if (node->next()->get_key() == key)
		{
			return node->next();
		}
		return tail;
	}

	void delete_node(node<Key, Value>* del_node)
	{
		auto lvl = del_node->get_level();
		for (int lvl_index = 0; lvl_index < lvl; ++lvl_index)
		{
			auto next_element = del_node->get_right_reference(lvl_index);
			auto prev_element = del_node->get_left_reference(lvl_index);
			next_element->set_left_reference(lvl_index, prev_element);
			prev_element->set_right_reference(lvl_index, next_element);
		}
		list_size -= 1;
		delete del_node;
	}

	void delete_list()
	{
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

	using iterator = node_iterator<Key, Value>;
	using const_iterator = const_node_iterator<Key, Value>;
	using reverse_iterator = reverse_node_iterator<Key, Value>;
	using const_reverse_iterator = reverse_const_node_iterator<Key, Value>;
	using value_type = std::pair<const Key, Value>;
	using size_type = size_t;

	explicit skip_list(const Compare& comp = Compare(), const Alloc& alloc = Alloc()) : compare(comp), allocator(alloc)
	{
		head = new node<Key, Value>;
		tail = new node<Key, Value>;
		list_lvl = 0;
		node<Key, Value>::bind_node(head, tail, Max_Level);
	}

	~skip_list()
	{
		delete_list();
	}

	skip_list(const skip_list& another) noexcept : compare(another.compare), allocator(another.allocator),
		list_size(another.list_size), list_lvl(another.list_lvl)
	{
		delete_list();
		head = new node<Key, Value>;
		tail = new node<Key, Value>;
		for (auto list_node : another)
		{
			insert(std::make_pair((*list_node).get_key(), (*list_node).get_value()));
		}
	}

	skip_list(skip_list&& another) noexcept : compare(std::move(another.compare)), allocator(std::move(another.allocator)),
		list_size(another.list_size), list_lvl(another.list_lvl)
	{
		delete_list();
		head = another.head;
		another.head = nullptr;
		tail = another.tail;
		another.tail = nullptr;
		another.list_lvl = 0;
		another.list_size = 0;
	}

	skip_list& operator=(const skip_list& another)//!!
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
			insert(std::make_pair((*list_node).get_key(), (*list_node).get_value()));//!!
		}
		return *this;
	}

	skip_list& operator=(skip_list&& another) noexcept
	{//!!
		compare = std::move(another.compare);
		allocator = std::move(another.allocator);
		list_size = another.list_size;
		list_lvl = another.list_lvl;
		delete_list();
		head = another.head;
		another.head = nullptr;
		tail = another.tail;
		another.tail = nullptr;
		return *this;
	}

	iterator begin()
	{
		return node_iterator<Key, Value>(head, tail, head->get_right_reference(0));
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_node_iterator<Key, Value>(head, tail, head->get_right_reference(0));
	}

	iterator end()
	{
		return node_iterator<Key, Value>(head, tail, tail);
	}

	[[nodiscard]] const_iterator end() const
	{
		return const_node_iterator<Key, Value>(head, tail, tail);
	}

	[[nodiscard]] bool empty() const
	{
		return list_size == 0;
	}

	[[nodiscard]] size_t size() const { return list_size; }

	Value& operator[](const Key& key)
	{
		auto searched_key = search_key(key);
		if (searched_key == tail)
		{
			Value new_value{};
			auto new_node = insert(std::make_pair(key, new_value));
			return (*new_node.first).get_value();
		}
		auto& value = searched_key->get_value();
		return value;
	}

	Value& at(const Key& key)
	{
		auto searched_key = search_key(key);
		auto& value = searched_key->get_value();
		return value;
	}

	[[nodiscard]] const Value& at(const Key& key) const
	{
		auto searched_key = search_key(key);
		const Value& value = searched_key->get_value();
		return value;
	}

	std::pair<iterator, bool> insert(const std::pair<const Key, Value>& value_nods)
	{
		auto [key, value] = value_nods;
		size_t level = random_level(Max_Level);
		node<Key, Value>* new_node = nullptr;
		if (list_size)
		{
			auto updated_nods = search_key_storing_past_elements(key, level);
			auto element = updated_nods[0]->next();
			if (element->get_key() == key)
			{
				element->set_value(value);
				return std::pair<iterator, bool>(node_iterator<Key, Value>(head, tail, element), false);
			}

			new_node = new node<Key, Value>(key, value, level);
			const size_t update_nods_size = updated_nods.size() - 1;
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
			new_node = new node<Key, Value>(key, value, level);
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
		return std::pair<iterator, bool>(node_iterator<Key, Value>(head, tail, new_node), true);
	}

	void erase(iterator position)
	{
		delete_node(*position);
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

	void erase(iterator& first, iterator& last)
	{
		for (auto iter = first; iter != last; ++iter)
		{
			if (iter != last)
			{
				erase(iter++);
			}
		}
		if (first != last)
		{
			erase(last);
		}
		erase(first);
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
		auto del_node = head->next();
		if (del_node == tail)
		{
			return;
		}
		auto next_node = del_node->next();
		while (del_node != tail)
		{
			delete del_node;
			del_node = next_node;
			next_node = next_node->next();
		}
		list_size = 0;
		list_lvl = 0;
		node<Key, Value>::bind_node(head, tail, Max_Level);
	}

	iterator find(const Key& key)
	{
		auto searched_node = search_key(key);
		if(searched_node == tail)
		{
			return iterator(tail, tail, tail);
		}
		if(searched_node->get_key() == key)
		{
			return iterator(head, tail, searched_node);
		}
		return iterator(tail, tail, tail);
	}
	[[nodiscard]] const_iterator find(const Key& key) const
	{
		auto searched_node = search_key(key);
		if (searched_node == tail)
		{
			return const_iterator(tail, tail, tail);
		}
		if (searched_node->get_key() == key)
		{
			return const_iterator(head, tail, searched_node);
		}
		return const_iterator(tail, tail, tail);
	}
	

	reverse_iterator rbegin();
	reverse_iterator rend();
	const_reverse_iterator rbegin() const;
	const_reverse_iterator rend() const;

	size_t count(const Key& key) const;

};

//template <typename K, typename V, typename C, typename A>
//inline bool operator==(const skip_list<K, V, C, A>& x, const skip_list<K, V, C, A>& y) {
//	// ....
//}
//
//template <typename K, typename V, typename C, typename A>
//inline bool operator!=(const skip_list<K, V, C, A>& x, const skip_list<K, V, C, A>& y) {
//	// ....
//}