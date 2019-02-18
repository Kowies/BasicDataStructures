#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include <list>
#include <iostream>
#include <functional>

namespace aisdi
{

template <typename KeyType, typename ValueType>
class HashMap
{
public:
  using key_type = KeyType;
  using mapped_type = ValueType;
  using value_type = std::pair<const key_type, mapped_type>;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;

  class ConstIterator;
  class Iterator;
  using iterator = Iterator;
  using const_iterator = ConstIterator;

private:

  size_type size_;

  //The table is rehashed when its size_ exceeds this threshold.
  //Value of this field is (int)(capacity * loadFactor)
  size_type threshold_;
  size_type capacity_;

  double loadFactor_;

  std::list<value_type> *bucket_; //if capacity_ == 0 operator new was not used

public:

  void print(std::ostream& out) const
  {
    out <<"Size: " << size_ << " Capacity: " << capacity_ /*<< " Bucket: "<< bucket_ */<< "\n";
    for (const auto& x: *this)
      out << "Key: " << x.first << " Value: " << x.second << "\n";

    out << std::endl;
  }


  HashMap(size_type capacity = 0, double loadFactor = 0.75): size_(0), threshold_(capacity * loadFactor),
    capacity_(capacity), loadFactor_(loadFactor)
  {
    if (loadFactor_ <= 0.0)
        throw std::logic_error(" HashMap(size_type capacity = 0, double loadFactor = 0.75)");

    if (capacity != 0)
      bucket_ = new std::list<value_type>[capacity_];
  }

  HashMap(std::initializer_list<value_type> list) : HashMap(16, 0.75)
  {
    for(const auto& iter : list)
      operator[](iter.first) = iter.second;
  }

  HashMap(const HashMap& other): HashMap(other.capacity_, other.loadFactor_)
  {
    for(const auto& iter : other)
      operator[](iter.first) = iter.second;
  }

  HashMap(HashMap&& other): HashMap()
  {
    std::swap(size_, other.size_);
    std::swap(threshold_, other.threshold_);
    std::swap(capacity_, other.capacity_);
    std::swap(loadFactor_, other.loadFactor_);
    std::swap(bucket_, other.bucket_);
  }

  HashMap& operator=(HashMap&& other)
  {
    if (this == &other)
      return *this;

    std::swap(size_, other.size_);
    std::swap(threshold_, other.threshold_);
    std::swap(capacity_, other.capacity_);
    std::swap(loadFactor_, other.loadFactor_);
    std::swap(bucket_, other.bucket_);
    return *this;
  }

  HashMap& operator=(const HashMap& other)
  {
    if (this == &other)
      return *this;

    HashMap temp = other;
    *this = std::move(temp);
    return *this;
  }

  ~HashMap()
  {
    if (capacity_ != 0)
      delete [] bucket_;
  }

  bool isEmpty() const
  {
    return (size_ == 0);
  }

  mapped_type& operator[](const key_type& key)
  {
    if (size_ == threshold_)
      rehash();

    return atWithoutRehash(key);
  }

  const mapped_type& valueOf(const key_type& key) const
  {
    auto iter = find(key);

    if (iter == end())
      throw std::out_of_range("const mapped_type& valueOf(const key_type& key) const");

    return iter->second;
  }

  mapped_type& valueOf(const key_type& key)
  {
    auto iter = find(key);

    if (iter == end())
      throw std::out_of_range("mapped_type& valueOf(const key_type& key)");

    return iter->second;
  }


  const_iterator find(const key_type& key) const
  {
    if (capacity_ == 0)
      return end();

    size_type hashValue = hash(key);

    std::list<value_type>& suspectList = bucket_[hashValue];

    for (auto iter = suspectList.begin(); iter != suspectList.end(); ++iter)
    {
      if (iter->first == key)
        return ConstIterator(this, hashValue, iter) ;
    }

    return end();
  }

  iterator find(const key_type& key)
  {
    if (capacity_ == 0)
      return end();

    size_type hashValue = hash(key);

    std::list<value_type>& suspectList = bucket_[hashValue];

    for (auto iter = suspectList.begin(); iter != suspectList.end(); ++iter)
    {
      if (iter->first == key)
        return ConstIterator(this, hashValue, iter) ;
    }

    return end();
  }

  void remove(const key_type& key)
  {
    iterator it = find(key);

    if (it == end())
      throw std::out_of_range("void remove(const const_iterator& it)");

    remove(it);
  }

  void remove(const const_iterator& it)
  {
    if (it == end())
      throw std::out_of_range("void remove(const const_iterator& it)");

    auto hashMapPtrFromIter = it.hashMapPtr_;

    if (hashMapPtrFromIter != this)
      throw std::logic_error("void remove(const const_iterator& it)");

    size_type arrayIndexFromIter = it.arrayIndex_;
    auto listIteratorFromIter = it.listIterator_;

    bucket_[arrayIndexFromIter].erase(listIteratorFromIter);
    --size_;
  }

  size_type getSize() const
  {
    return size_;
  }

  bool operator==(const HashMap& other) const
  {
    if (size_ != other.size_)
      return false;

    for (auto const& x: other)
    {
      if (!isHere(x))
        return false;
    }

    return true;
  }

  bool operator!=(const HashMap& other) const
  {
    return !(*this == other);
  }

  iterator begin()
  {
    if (size_ == 0)
      return end();

    for (size_type i = 0; i < capacity_; ++i)
    {
      if (bucket_[i].size() != 0)
        return Iterator(this, i, bucket_[i].begin() );
    }

    throw std::logic_error("No way: iterator begin()");
  }

  iterator end()
  {
    return Iterator(this, capacity_);
  }

  const_iterator cbegin() const
  {
    if (size_ == 0)
      return end();

    for (size_type i = 0; i < capacity_; ++i)
    {
      if (bucket_[i].size() != 0)
        return ConstIterator(this, i, bucket_[i].begin() );
    }

    throw std::logic_error("No way: iterator begin()");
  }

  const_iterator cend() const
  {
    return ConstIterator(this, capacity_);
  }

  const_iterator begin() const
  {
    return cbegin();
  }

  const_iterator end() const
  {
    return cend();
  }

private:

  size_type hash(const key_type& key) const
  {
    return (std::hash<key_type>{}(key) ) % capacity_;
  }

  void rehash()
  {
    size_type newCapacity;
    double newLoadFactor;

    if (capacity_ < 8)
    {
      newCapacity = 16;
      newLoadFactor = loadFactor_;
    }
    else
    {
      newCapacity = 2 * capacity_;
      newLoadFactor = loadFactor_;
    }

    HashMap <KeyType, ValueType> newHashMap(newCapacity, newLoadFactor);

    for (const auto &x: *this)
      newHashMap.atWithoutRehash(x.first) = x.second;

    *this = std::move(newHashMap);
  }

  mapped_type& atWithoutRehash(const key_type& key)
  {
    int valueOfHash = hash(key);

    for (auto& x: bucket_[valueOfHash]) //can I change?
    {
      if (x.first == key)
        return x.second;
    }

    bucket_[valueOfHash].push_front(value_type{key, mapped_type() } );

    ++size_;

    return bucket_[valueOfHash].begin()->second;
  }

  bool isHere(const value_type & value) const
  {
    auto iter = find(value.first);

    if (iter == end() )
      return false;

    if (*iter != value)
      return false;

    return true;
  }

};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::const_reference;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename HashMap::value_type;
  using pointer = const typename HashMap::value_type*;

  friend void HashMap::remove(const const_iterator& it);

private:

  const HashMap *hashMapPtr_;
  size_type arrayIndex_;
  typename std::list<value_type>::iterator listIterator_;


public:

  ConstIterator(const HashMap *hashMapPtr, size_type arrayIndex, typename std::list<value_type>::iterator listIterator):
    hashMapPtr_(hashMapPtr), arrayIndex_(arrayIndex), listIterator_(listIterator)
  {}

  //construcktor only for end()
  ConstIterator(const HashMap *hashMapPtr, size_type arrayIndex):
    hashMapPtr_(hashMapPtr), arrayIndex_(arrayIndex)
  {}

  explicit ConstIterator()
  {}

  ConstIterator(const ConstIterator& other):
    hashMapPtr_(other.hashMapPtr_), arrayIndex_(other.arrayIndex_), listIterator_(other.listIterator_)
  {}

  ConstIterator& operator++()
  {
    if (*this == hashMapPtr_->end())
      throw std::out_of_range("ConstIterator& operator++()");

    ++listIterator_;

    std::list<value_type> &actualList = hashMapPtr_->bucket_[arrayIndex_];

    if (listIterator_ != actualList.end())
      return *this;

    for (++arrayIndex_; arrayIndex_ < hashMapPtr_->capacity_; ++arrayIndex_)
    {
      std::list<value_type> &bucket = hashMapPtr_->bucket_[arrayIndex_];

      if (bucket.size() != 0)
      {
        listIterator_ = bucket.begin();
        return *this;
      }
    }

    return *this;
  }

  ConstIterator operator++(int)
  {
    ConstIterator temp = *this;
    ++(*this);
    return temp;
  }

  ConstIterator& operator--()
  {
    if (*this == hashMapPtr_->begin())
      throw std::out_of_range("ConstIterator& operator--()");

    std::list<value_type> &actualList = hashMapPtr_->bucket_[arrayIndex_];

    if (*this != hashMapPtr_->end() && listIterator_ != actualList.begin() )
    {
      --listIterator_;
      return *this;
    }

    for (--arrayIndex_; arrayIndex_ >= 0; --arrayIndex_)
    {
      std::list<value_type> &bucket = hashMapPtr_->bucket_[arrayIndex_];

      if (bucket.size() != 0)
      {
        listIterator_ = bucket.end();
        --listIterator_;
        return *this;
      }
    }

    throw std::runtime_error("If you see this contact with autor of code.");
  }

  ConstIterator operator--(int)
  {
    ConstIterator temp = *this;
    --(*this);
    return temp;
  }

  reference operator*() const
  {
    if (*this == hashMapPtr_->end())
      throw std::out_of_range("reference operator*() const");

    return *listIterator_;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  bool operator==(const ConstIterator& other) const
  {
    if (hashMapPtr_ != other.hashMapPtr_)
      return false;

    if (arrayIndex_ != other.arrayIndex_)
      return false;

    if (hashMapPtr_->capacity_ == arrayIndex_) //both are end()
      return true;

    if (listIterator_ != other.listIterator_)
      return false;

    return true;
  }

  bool operator!=(const ConstIterator& other) const
  {
    return !(*this == other);
  }
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::reference;
  using pointer = typename HashMap::value_type*;


  Iterator(const HashMap *hashMapPtr, size_type arrayIndex, typename std::list<value_type>::iterator listIterator):
   ConstIterator(hashMapPtr, arrayIndex, listIterator)
  {}

  //construcktor only for end()
  Iterator(const HashMap *hashMapPtr, size_type arrayIndex):
    ConstIterator(hashMapPtr, arrayIndex)
  {}

  explicit Iterator()
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

  pointer operator->() const
  {
    return &this->operator*();
  }

  reference operator*() const
  {
    // ugly cast, yet reduces code duplication.
    return const_cast<reference>(ConstIterator::operator*());
  }
};

}

#endif /* AISDI_MAPS_HASHMAP_H */
