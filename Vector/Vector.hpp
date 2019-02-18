#ifndef AISDI_LINEAR_VECTOR_H
#define AISDI_LINEAR_VECTOR_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <iostream>

namespace aisdi
{

template <typename Type>
class Vector
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
  char *buffer_ = nullptr;
  size_type size_ = 0;
  size_type capacity_ = 0;


  char* allocateLargerBuffer()
  {
    if (capacity_ == 0) //first allocation
      capacity_ = 8;
    else
      capacity_ = size_ << 1;

    char* newBuffer = new char[capacity_ * sizeof(value_type)];

    return newBuffer;
  }

public:

  void print(std::ostream &out)
  {
    out << "Size: " << size_ << " Capacity: " << capacity_ << std::endl;

    for (size_t i = 0; i < size_; ++i)
      out << i << ": " << (*this)[i] <<"\n";

    out << std::endl;
  }

  Vector() : buffer_(nullptr), size_(0), capacity_(0)
  {}

  reference operator[](unsigned int index)
  {
    return reinterpret_cast<pointer>(buffer_)[index];
  }

  Vector(std::initializer_list<Type> l)
  {
     
    for (auto iter = l.begin(); iter != l.end(); ++iter)
      ++size_;

    capacity_ = size_;
    if (capacity_ == 0)
      return;

    buffer_ = new char[capacity_ * sizeof(value_type) ];

    size_type i = 0;

    for (auto iter = l.begin(); iter != l.end(); ++iter, ++i)
      new (buffer_ + sizeof(value_type) * i)value_type(*iter);

  }

  Vector(const Vector& other)
  {
    size_ = other.getSize();

    capacity_ = size_;
    if (capacity_ == 0)
      return;

    buffer_ = new char[capacity_ * sizeof(value_type)];

    size_type i = 0;
    for (auto iter = other.begin() ; iter != other.end(); ++iter)
    {
      new (buffer_ + sizeof(value_type) * i)value_type(*iter);
      ++i;
    }

  }

  Vector(Vector&& other) : buffer_(other.buffer_), size_(other.size_), capacity_(other.capacity_)
  {
    other.capacity_ = 0;
  }

  ~Vector()
  {
    if (capacity_ == 0)
      return;

    for(size_type i = 0; i < getSize(); ++i)
      (reinterpret_cast<pointer>(buffer_)[i]).~value_type();

    delete [] buffer_;

  }

  Vector& operator=(const Vector& other)
  {
    if (this == &other)
      return *this;


    if (capacity_ != 0)
    {
      for(size_type i = 0; i < getSize(); ++i)
        (reinterpret_cast<pointer>(buffer_)[i]).~value_type();

      delete [] buffer_;
    }

    size_ = other.size_;
    capacity_ = size_;
    buffer_ = new char[capacity_ * sizeof(value_type) ];

    size_type i = 0;
    for (auto iter = other.begin(); iter != other.end(); ++iter)
    {
      new (buffer_ + sizeof(value_type) * i)value_type(*iter);
      ++i;
    }

    return *this;
  }

  Vector& operator=(Vector&& other)
  {

    if (capacity_ != 0)
    {
      for(size_type i = 0; i < getSize(); ++i)
        (reinterpret_cast<pointer>(buffer_)[i]).~value_type();

      delete [] buffer_;
    }

    size_ = other.size_;
    capacity_ = other.capacity_;
    buffer_ = other.buffer_;

    other.buffer_ = nullptr;
    other.capacity_ = 0;
    other.size_ = 0;

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
    if (size_ == capacity_)
    {
      char *newBuffer = allocateLargerBuffer();

      pointer bufferCasted = reinterpret_cast<pointer>(buffer_);
      pointer newBufferCasted = reinterpret_cast<pointer>(newBuffer);

      for (size_type i = 0; i < size_; ++i)
      {
        new (newBufferCasted + i)value_type(bufferCasted[i]);
        bufferCasted[i].~value_type();

        newBufferCasted[i] = bufferCasted[i];
      }

      if (capacity_ != 0)
        delete [] buffer_;

      buffer_ = newBuffer;
    }

    new (buffer_ + sizeof(value_type) * size_)value_type(item);
    ++size_;
  }

  void prepend(const Type& item)
  {
    char *newBuffer = allocateLargerBuffer();

    pointer bufferCasted = reinterpret_cast<pointer>(buffer_);
    pointer newBufferCasted = reinterpret_cast<pointer>(newBuffer);


    new (newBufferCasted)value_type(item);

    for (size_type i = 0; i < size_; ++i)
    {
      new (newBufferCasted + i + 1)value_type(bufferCasted[i]);
      bufferCasted[i].~value_type();
    }

    if (capacity_ != 0)
      delete [] buffer_;

    buffer_ = newBuffer;

    ++size_;
  }

  void insert(const const_iterator& insertPosition, const Type& item)
  {

    if (size_ == capacity_)
    {
      char *newBuffer = allocateLargerBuffer();
      difference_type distance = (insertPosition - begin() );
      pointer bufferCasted = reinterpret_cast<pointer>(buffer_);
      pointer newBufferCasted = reinterpret_cast<pointer>(newBuffer);

      for (difference_type i = 0; i < distance; ++i)
      {
        new (newBufferCasted + i)value_type(bufferCasted[i]);
        bufferCasted[i].~value_type();
      }

      new (newBufferCasted + distance)value_type(item);
      ++size_;

      for (size_type i = distance + 1; i < size_; ++i)
      {
        new (newBufferCasted + i)value_type(bufferCasted[i - 1]);
        bufferCasted[i - 1].~value_type();
      }
      bufferCasted[distance].~value_type();

      if (capacity_ != 0)
        delete [] buffer_;

      buffer_ = newBuffer;
    }
    else
    {
      difference_type distance = (insertPosition - begin() );
      pointer bufferCasted = reinterpret_cast<pointer>(buffer_);

      bufferCasted[distance].~value_type();
      new (bufferCasted + distance)value_type(item);
      ++size_;

      for (size_type i = distance + 1; i < size_; ++i)
      {
        new (bufferCasted + i)value_type(bufferCasted[i - 1]);
        bufferCasted[i - 1].~value_type();
      }
      bufferCasted[distance].~value_type();
    }
  }

  Type popFirst()
  {
    if (size_ == 0)
      throw std::logic_error("V popFirst");

    value_type tmp = *begin();
    erase(begin());
    return tmp;
  }

  Type popLast()
  {
    if (size_ == 0)
      throw std::logic_error("V popLast");

    pointer bufferCasted = reinterpret_cast<pointer>(buffer_);
    value_type tmp = bufferCasted[--size_];
    bufferCasted[size_].~value_type();
    return tmp;
  }

  void erase(const const_iterator& possition)
  {
    if (size_ == 0)
      throw std::out_of_range("V erase(i)");

    if(possition == end())
      throw std::out_of_range("V erase(i)");

    difference_type distance = possition - begin();


    pointer bufferCasted = reinterpret_cast<pointer>(buffer_);

    --size_;

    for (size_type i = distance; i < size_; ++i)
    {
      bufferCasted[i].~value_type();
      new (bufferCasted + i)value_type(bufferCasted[i + 1]);
    }

  }

  void erase(const const_iterator& firstIncluded, const const_iterator& lastExcluded)
  {

    if (firstIncluded == lastExcluded)
      return;

    if (size_ == 0)
      throw std::out_of_range("V erase(i, i)");

    //difference_type distanceToLastErased = (lastExcluded - begin() - 1);
    difference_type distanceToFirstErased = firstIncluded - begin();
    difference_type howManyErased = (lastExcluded - firstIncluded);

    pointer bufferCasted = reinterpret_cast<pointer>(buffer_);

    for (iterator iter = firstIncluded; iter != lastExcluded; ++iter)
    {
      (*iter).~value_type();
    }

    size_type i = 0;
    
    for (iterator iter = lastExcluded ; iter != end(); ++iter)
    {
      new (bufferCasted + distanceToFirstErased + i)value_type(*iter);
      (*iter).~value_type();
      ++i;
    }

    size_ -= howManyErased;

  }

  iterator begin()
  {
    return iterator(this, buffer_);
  }

  iterator end()
  {
    return iterator(this, buffer_ + sizeof(value_type) * size_);
  }

  const_iterator cbegin() const
  {
    return const_iterator(this, buffer_);
  }

  const_iterator cend() const
  {
    return const_iterator(this, buffer_ + sizeof(value_type) * size_);
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
class Vector<Type>::ConstIterator
{
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename Vector::value_type;
  using difference_type = typename Vector::difference_type;
  using pointer = typename Vector::const_pointer;
  using reference = typename Vector::const_reference;

private:
  const Vector *vect_;
  char *ptr_;


public:

  difference_type operator-(const ConstIterator& iter) const
  {
    difference_type distance = (this->ptr_ - iter.ptr_) / sizeof(value_type);
    return distance;
  }

  explicit ConstIterator()
  {}

  ConstIterator(const Vector<value_type> *vect, char *ptr) : vect_(vect), ptr_(ptr)
  {}

  ConstIterator(const Vector<value_type> *vect, const char *ptr) : vect_(vect)
  {
    ptr_ = const_cast<char *> (ptr);
  }

  reference operator*() const
  {
    if (ptr_ == vect_->end().ptr_)
      throw std::out_of_range("");

    return *(reinterpret_cast<pointer>(ptr_) );
  }

  ConstIterator& operator++()
  {
    if (ConstIterator(vect_, ptr_) == vect_->end())
      throw std::out_of_range("");

    ptr_ += sizeof(value_type);
    return *this;
  }

  ConstIterator operator++(int)
  {
    if (ptr_ == vect_->end().ptr_)
      throw std::out_of_range("");

    ConstIterator tmp(this->vect_, ptr_);
    ptr_ += sizeof(value_type);
    return tmp;
  }

  ConstIterator& operator--()
  {
    if (ptr_ == vect_->begin().ptr_)
      throw std::out_of_range("Empty");

    ptr_ -= sizeof(value_type);
    return *this;
  }

  ConstIterator operator--(int)
  {
    if (ptr_ == vect_->begin().ptr_)
      throw std::out_of_range("");

    ConstIterator tmp(this->vect_, ptr_);
    ptr_ -= sizeof(value_type);
    return tmp;
  }

  ConstIterator operator+(difference_type d) const
  {
    if (ptr_ + d > vect_->end().ptr_ || ptr_ + d < vect_->begin().ptr_)
      throw std::out_of_range("");

    char * tmp = ptr_ + sizeof(value_type) * d;
    return ConstIterator(this->vect_,tmp);
  }

  ConstIterator operator-(difference_type d) const
  {
    if (ptr_ - d > vect_->end().ptr_ || ptr_ - d < vect_->begin().ptr_)
      throw std::out_of_range("");

    char * tmp = ptr_ - sizeof(value_type) * d;
    return ConstIterator(this->vect_,tmp);
  }

  bool operator==(const ConstIterator& other) const
  {
    if (ptr_ == other.ptr_)
      return true;

    return false;
  }

  bool operator!=(const ConstIterator& other) const
  {
    if (ptr_ == other.ptr_)
      return false;

    return true;
  }
};

template <typename Type>
class Vector<Type>::Iterator : public Vector<Type>::ConstIterator
{
public:
  using pointer = typename Vector::pointer;
  using reference = typename Vector::reference;

  explicit Iterator()
  {}

  Iterator(const ConstIterator& other)
    : ConstIterator(other)
  {}

  Iterator(const Vector<value_type> *vect, char *ptr) : ConstIterator(vect, ptr)
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



#endif // AISDI_LINEAR_VECTOR_H
