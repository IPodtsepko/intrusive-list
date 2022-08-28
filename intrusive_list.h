#pragma once

#include <iostream>
#include <iterator>

namespace intrusive {
struct default_tag;

struct node {
  node* next = nullptr;
  node* prev = nullptr;

  node() = default;
  node(node&&);
  ~node();

  node& operator=(node&&);

  void unlink();
  void link(node*, node*);
  void splice(node* first, node* last);

private:
  void three_swap(node*& a, node*& b, node*& c);
};

template <typename Tag = default_tag>
struct list_element : private node {
  template <typename T, typename U>
  friend struct list;
};

template <typename T, typename Tag = default_tag>
struct list {
  static_assert(std::is_convertible_v<T&, list_element<Tag>&>,
                "value type is not convertible to list_element");

private:
  template <class U>
  struct base_iterator {
    friend struct list<T, Tag>;

    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = U;
    using reference = U&;
    using pointer = U*;

    base_iterator() = default;

    template <typename V, typename = std::enable_if_t<std::is_const_v<U> &&
                                                      !std::is_const_v<V>>>
    base_iterator(base_iterator<V> const& it) : p(it.p) {}

    pointer operator->() const {
      auto tmp = static_cast<list_element<Tag> *>(p);
      return static_cast<U*>(tmp);
    }

    reference operator*() const {
      auto tmp = static_cast<list_element<Tag>*>(p);
      return static_cast<U&>(*tmp);
    }

    base_iterator& operator++() {
      p = p->next;
      return *this;
    }

    base_iterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    base_iterator& operator--() {
      p = p->prev;
      return *this;
    }

    base_iterator operator--(int) {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    friend bool operator==(const base_iterator& lhs, const base_iterator& rhs) {
      return lhs.p == rhs.p;
    }

    friend bool operator!=(const base_iterator& lhs, const base_iterator& rhs) {
      return !(lhs == rhs);
    }

  private:
    node* p;

    explicit base_iterator(node* ptr) : p(ptr) {}
  };

public:
  using iterator = base_iterator<T>;
  using const_iterator = base_iterator<T const>;

  list() noexcept {
    init_fake_for_empty();
  }

  list(list const&) = delete;

  list(list&& other) noexcept : list() {
    *this = std::move(other);
  }

  ~list() {
    clear();
  }

  list& operator=(list const&) = delete;
  list& operator=(list&& other) noexcept {
    clear();
    if (!other.empty()) {
      fake = std::move(other.fake);
      other.init_fake_for_empty();
    }
    return *this;
  }

  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  void push_back(T& value) noexcept {
    insert(end(), value);
  }

  void pop_back() noexcept {
    erase(std::prev(end()));
  }

  T& back() noexcept {
    return *(std::prev(end()));
  }

  T const& back() const noexcept {
    return *(std::prev(end()));
  }

  void push_front(T& value) noexcept {
    insert(begin(), value);
  }

  void pop_front() noexcept {
    erase(begin());
  }

  T& front() noexcept {
    return *begin();
  }

  T const& front() const noexcept {
    return *begin();
  }

  bool empty() const noexcept {
    return fake.next == &fake;
  }

  iterator begin() noexcept {
    return iterator(fake.next);
  }

  const_iterator begin() const noexcept {
    return const_iterator(fake.next);
  }

  iterator end() noexcept {
    return iterator(&fake);
  }

  const_iterator end() const noexcept {
    return const_iterator(&fake);
  }

  iterator insert(const_iterator pos, T& value) noexcept {
    auto new_node = static_cast<list_element<Tag>*>(&value);
    new_node->link(pos.p->prev, pos.p);
    return iterator(new_node);
  }

  iterator erase(const_iterator pos) noexcept {
    iterator it(pos.p->prev);
    static_cast<list_element<Tag>*>(pos.p)->unlink();
    return std::next(it);
  }

  void splice(const_iterator pos, list&, const_iterator first,
              const_iterator last) noexcept {
    pos.p->splice(first.p, last.p);
  }

private:
  mutable node fake;

  void init_fake_for_empty() {
    fake.next = fake.prev = &fake;
  }
};
} // namespace intrusive