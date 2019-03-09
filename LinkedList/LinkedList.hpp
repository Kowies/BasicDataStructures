#ifndef AISDI_LINEAR_LINKEDLIST_H
#define AISDI_LINEAR_LINKEDLIST_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <iostream>

namespace aisdi
{

template <typename Type>
class LinkedList
{
public:
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;
  using value_type = Type;
  using pointer = Type*;
  using reference = Type&;
  using const_pointer = const Type*;
  using const_reference = const Type&;

  class ConstIterator;
  class Iterator;
  using iterator = Iterator;
  using const_iterator = ConstIterator;

private:

  struct Node
  {
    char data[sizeof(value_type)];
    Node *next;
    Node *prev;

    value_type &value = *reinterpret_cast <pointer> (data);

    Node() : next(this), prev(this)
    {}

    Node(const_reference x):  next(this), prev(this)
    {
      new (data) value_type(x);
    }

    ~Node()
    {
      if (next == this)
        return;

      value.~value_type();
    }

    void clean(){
      next = this;
      prev = this;
    }

    bool isEmpty()
    {
      if (next == this)
        return true;

      return false;
    }

  };


  Node watchman_;

  size_type size_ = 0;

  Node* & head = watchman_.next;
  Node* & tail = watchman_.prev;

public:

  void print(std::ostream &out) const
  {
    out << "Size: " << size_ << std::endl;

    size_type i = 0;
    for (iterator iter = begin(); iter != end(); ++iter)
    {
      out << i++ << ": " << *iter << '\n';
    }

    out << std::endl;
  }

  LinkedList(): size_(0)
  {}

  LinkedList(std::initializer_list<Type> l)
  {
    for (auto iter = l.begin(); iter != l.end(); ++iter)
      append(*iter);
  }

  LinkedList(const LinkedList& other)
  {
    for (auto iter = other.begin(); iter != other.end(); ++iter)
      append(*iter);
  }

  LinkedList(LinkedList&& other)
  {
    other.head->prev = &watchman_;
    other.tail->next = &watchman_;
    watchman_.next = other.head;
    watchman_.prev = other.tail;

    size_ = other.size_;

    other.size_ = 0;
    other.watchman_.clean();
  }

  ~LinkedList()
  {
    if (size_ == 0)
      return;

    erase(begin(), end());
  }

  LinkedList& operator=(const LinkedList& other)
  {
    if (this == &other)
      return *this;

    if (size_ != 0)
    {
      erase(begin(), end());
    }

    for (iterator iter = other.begin(); iter != other.end(); ++iter)
      append(*iter);

    return *this;
  }


  LinkedList& operator=(LinkedList&& other)
  {

    if (size_ != 0)
    {
      erase(begin(), end());
    }

    other.head->prev = &watchman_;
    other.tail->next = &watchman_;
    watchman_.next = other.head;
    watchman_.prev = other.tail;

    size_ = other.size_;

    other.size_ = 0;
    other.watchman_.clean();

    return *this;
  }

  bool isEmpty() const
  {
    if (size_ == 0)
      return true;

    return false;
  }

  size_type getSize() const
  {
    return size_;
  }

  void append(const Type& item)
  {

    Node *tmp = new Node(item);
    watchman_.prev->next = tmp;
    tmp->prev = watchman_.prev;
    watchman_.prev = tmp;
    watchman_.prev->next = &watchman_;

    ++size_;

  }

  void prepend(const Type& item)
  {
    Node *tmp = new Node(item);
    watchman_.next->prev = tmp;
    tmp->next = watchman_.next;
    watchman_.next = tmp;
    watchman_.next->prev = &watchman_;

    ++size_;
  }

  void insert(const const_iterator& insertPosition, const Type& item)
  {
    Node *afterNew = insertPosition.ptr_;
    Node *beforeNew = afterNew->prev;

    Node *newNode = new Node(item);

    beforeNew->next = newNode;
    newNode->prev = beforeNew;

    afterNew->prev = newNode;
    newNode->next = afterNew;

    ++size_;
  }

  Type popFirst()
  {
    if (size_ == 0)
      throw std::out_of_range("Empty1");

    Type tmp = watchman_.next->value;

    watchman_.next = watchman_.next->next;
    delete watchman_.next->prev;
    watchman_.next->prev = &watchman_;

    --size_;

    return tmp;
  }

  Type popLast()
  {
    if (size_ == 0)
      throw std::out_of_range("Empty2");

    Type tmp = watchman_.prev->value;

    watchman_.prev = watchman_.prev->prev;
    delete watchman_.prev->next;
    watchman_.prev->next = &watchman_;

    --size_;

    return tmp;
  }

  void erase(const const_iterator& possition)
  {
    if (size_ == 0)
      throw std::out_of_range("L erase(i)");

    if(possition == end())
      throw std::out_of_range("L erase(i)");

     Node *ptr_ = possition.ptr_;

     ptr_->next->prev = ptr_->prev;
     ptr_->prev->next = ptr_->next;

     delete ptr_;
     --size_;
  }

  void erase(const const_iterator& firstIncluded, const const_iterator& lastExcluded)
  {

    if (firstIncluded == lastExcluded)
      return;

    if (size_ == 0)
      throw std::out_of_range("L erase(i)");

    Node *beforeFirstIncludedPtr = firstIncluded.ptr_->prev;
    Node *lastExcludedPtr = lastExcluded.ptr_;

    const_iterator iter = firstIncluded;

    int howManyDeleted = 0;
    while (iter != lastExcluded)
    {
      iterator tmp = iter++;
      delete tmp.ptr_;
      ++howManyDeleted;
    }

    size_ -= howManyDeleted;
    beforeFirstIncludedPtr->next = lastExcludedPtr;
    lastExcludedPtr->prev = beforeFirstIncludedPtr;
  }

  iterator begin()
  {
    return iterator(this, watchman_.next);
  }

  iterator end()
  {
    return iterator(this, &watchman_);
  }

  const_iterator cbegin() const
  {

    return const_iterator(this, watchman_.next);
  }

  const_iterator cend() const
  {
    auto x = const_iterator(this, &watchman_);
    return x;
  }

  const_iterator begin() const
  {
    return cbegin();
  }

  const_iterator end() const
  {
    return cend();
  }

};

template <typename Type>
class LinkedList<Type>::ConstIterator
{
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename LinkedList::value_type;
  using difference_type = typename LinkedList::difference_type;
  using pointer = typename LinkedList::const_pointer;
  using reference = typename LinkedList::const_reference;

private:
  const LinkedList *llist_;
  Node *ptr_;


  friend void LinkedList <Type>::insert(const const_iterator&, const Type&);
  friend void LinkedList <Type>::erase(const const_iterator&, const const_iterator&);
  friend void LinkedList <Type>::erase(const const_iterator&);

public:

  explicit ConstIterator()
  {}

  ConstIterator(const LinkedList *llist, Node *ptr) : llist_(llist), ptr_(ptr)
  {}

  ConstIterator(const LinkedList *llist, const Node *ptr) : llist_(llist)
  {
    ptr_ = const_cast<Node*> (ptr);
  }


  reference operator*() const
  {
    if (ConstIterator(llist_, ptr_) == llist_->end() )
      throw std::out_of_range("7");

    return ptr_->value;
  }

  ConstIterator& operator++()
  {
    if (ConstIterator(llist_, ptr_) == llist_->end() )
      throw std::out_of_range("5");

    ptr_ = ptr_->next;
    return *this;
  }

  ConstIterator operator++(int)
  {
    if (ConstIterator(llist_, ptr_) == llist_->end() )
      throw std::out_of_range("5");

    ConstIterator tmpConstIterator = ConstIterator(llist_, ptr_);
    ptr_ = ptr_->next;
    return tmpConstIterator;
  }

  ConstIterator& operator--()
  {
    if (ConstIterator(llist_, ptr_) == llist_->begin() )
      throw std::out_of_range("6");

    ptr_ = ptr_->prev;
    return *this;
  }

  ConstIterator operator--(int)
  {
    if (ConstIterator(llist_, ptr_) == llist_->begin() )
      throw std::out_of_range("6");

    ConstIterator tmpConstIterator = ConstIterator(llist_, ptr_);
    ptr_ = ptr_->prev;
    return tmpConstIterator;
  }

  ConstIterator operator+(difference_type d) const
  {
    const_iterator result = (*this);

    for (difference_type i = 0; i < d; ++i)
    {
      if (result == llist_->end() )
      {
        throw std::out_of_range("5");
      }
        ++result;
    }

    return result;
  }

  ConstIterator operator-(difference_type d) const
  {
    const_iterator result = (*this);

    for (difference_type i = 0; i < d; ++i)
    {
      if (result == llist_->begin() )
      {
        throw std::out_of_range("5");
      }

        --result;
    }
    return result;
  }

  bool operator==(const ConstIterator& other) const
  {
    if (this->ptr_ == other.ptr_)
      return true;

    return false;
  }

  bool operator!=(const ConstIterator& other) const
  {
    return !(*this == other);
  }
};

template <typename Type>
class LinkedList<Type>::Iterator : public LinkedList<Type>::ConstIterator
{
public:
  using pointer = typename LinkedList::pointer;
  using reference = typename LinkedList::reference;

  explicit Iterator()
  {}

  Iterator(const LinkedList<value_type> *llist, const Node *ptr) : ConstIterator(llist, ptr)
  {}

  Iterator(const ConstIterator& other)
    : ConstIterator(other)
  {}

  Iterator& operator++()
  {
    ConstIterator::operator++();
    return *this;
  }

  Iterator operator++(int)
  {
    auto result = *this;
    ConstIterator::operator++();
    return result;
  }

  Iterator& operator--()
  {
    ConstIterator::operator--();
    return *this;
  }

  Iterator operator--(int)
  {
    auto result = *this;
    ConstIterator::operator--();
    return result;
  }

  Iterator operator+(difference_type d) const
  {
    return ConstIterator::operator+(d);
  }

  Iterator operator-(difference_type d) const
  {
    return ConstIterator::operator-(d);
  }

  reference operator*() const
  {
    // ugly cast, yet reduces code duplication.
    return const_cast<reference>(ConstIterator::operator*());
  }
};

}

#endif // AISDI_LINEAR_LINKEDLIST_H
