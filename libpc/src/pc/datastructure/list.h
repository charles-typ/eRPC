#ifndef PC_API_LIST_H
#define PC_API_LIST_H
namespace pc {
namespace datastructure {
namespace list {

template<class T>
class List;

class Iterator;

class Node {
  friend class Iterator;
  template<class T> friend
  class List;

 protected:
  Node *next_, *prev_;

  void push_back(Node *n) {
    n->next_ = this;
    n->prev_ = prev_;
    prev_->next_ = n;
    prev_ = n;
  }

  void unlink() {
    Node *next = next_, *prev = prev_;
    next->prev_ = prev;
    prev->next_ = next;
    next_ = this;
    prev_ = this;
  }

 public:
  Node()
      : next_(this), prev_(this) {}

  ~Node() { unlink(); }
};

class Iterator {
 protected:
  Node *node_;

  Iterator(Node *node)
      : node_(node) {}

 public:
  Iterator &operator++() {
    node_ = node_->next_;
    return *this;
  }

  bool operator==(Iterator b) const { return node_ == b.node_; }
  bool operator!=(Iterator b) const { return node_ != b.node_; }
};

template<class T>
class List {
  class NodeT : public Node {
    friend class List<T>;
    T value_;
    NodeT(T t) : value_(t) {}
  };

  template<class U>
  class IteratorT : public Iterator {
    friend class List<T>;
    NodeT *node() const { return static_cast<NodeT *>(node_); }
   public:
    U &operator*() const { return node()->value_; }
    U *operator->() const { return &node()->value_; }
    operator IteratorT<U const>() const { return node_; }
    IteratorT(Node *node) : Iterator{node} {}
  };

  Node list_;

 public:
  using iterator = IteratorT<T>;
  using const_iterator = IteratorT<T const>;

  List() {}
  List(iterator begin, iterator end) {
    for (auto itr = begin; itr != end; ++itr) {
      this->push_back(*itr);
    }
  }

  ~List() { clear(); }

  bool empty() const { return list_.next_ == &list_; }

  iterator begin() { return list_.next_; }
  iterator end() { return &list_; }

  void push_back(T t) { list_.push_back(new NodeT(t)); }
  void erase(const_iterator i) { delete i.node(); }

  iterator find(T t) {
    for (auto itr = begin(); itr != end(); ++itr) {
      if (*itr == t) return itr;
    }

    return end();
  }

  void clear() {
    while (!empty())
      erase(begin());
  }
};

}
}
}

#endif //PC_API_LIST_H
