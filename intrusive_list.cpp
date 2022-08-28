#include "intrusive_list.h"

intrusive::node::node(intrusive::node&& src) {
  *this = std::move(src);
}

intrusive::node::~node() {
  unlink();
}

intrusive::node& intrusive::node::operator=(intrusive::node&& src) {
  if (this != &src) {
    link(src.prev, src.next);
    src.prev = nullptr;
    src.next = nullptr;
  }
  return *this;
}

void intrusive::node::unlink() {
  if (next == nullptr)
    return;

  next->prev = prev;
  prev->next = next;

  prev = nullptr;
  next = nullptr;
}

void intrusive::node::link(intrusive::node* prev, intrusive::node* next) {
  unlink();

  this->prev = prev;
  this->next = next;

  prev->next = this;
  next->prev = this;
}

void intrusive::node::splice(intrusive::node* first, intrusive::node* last) {
  if (first == last) {
    return;
  }
  three_swap(prev->next, first->prev->next, last->prev->next);
  three_swap(prev, last->prev, first->prev);
}

void intrusive::node::three_swap(intrusive::node*& a, intrusive::node*& b,
                                 intrusive::node*& c) {
  node* tmp = a;
  a = b;
  b = c;
  c = tmp;
}
