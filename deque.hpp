#include <cstring>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T>
class Deque {
 public:
  template <bool IsConst>
  struct DequeIterator;
  using iterator = DequeIterator<false>;
  using const_iterator = DequeIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Deque();
  Deque(const Deque& other);

  Deque(size_t count, const T& value = T());

  void add_one(const T& value);

  Deque& operator=(const Deque& rhs);

  void clear();

  size_t size() const;

  T& operator[](size_t index);

  const T& operator[](size_t index) const;

  T& at(size_t index);

  const T& at(size_t index) const;

  void push_back(const T& value);

  void push_front(const T& value);

  void pop_back();

  void pop_front();

  bool empty() const;

  template <bool IsConst>
  struct DequeIterator {
    operator DequeIterator<true>() const;

    using difference_type = std::ptrdiff_t;
    using value_type = typename std::conditional<IsConst, const T, T>::type;
    using pointer = typename std::conditional<IsConst, const T*, T*>::type;
    using reference = typename std::conditional<IsConst, const T&, T&>::type;
    using iterator_category = std::random_access_iterator_tag;

    DequeIterator(size_t segment, size_t index)
        : segment(segment), index(index) {}
    DequeIterator(size_t segment, size_t index, Deque<T>* p_deque)
        : segment(segment), index(index), deque(p_deque) {}

    DequeIterator& operator++();

    DequeIterator& operator--();

    DequeIterator operator++(int);

    DequeIterator operator--(int);

    DequeIterator& operator+=(size_t ind);

    DequeIterator& operator-=(size_t ind);

    DequeIterator operator+(size_t ind) const;

    DequeIterator operator-(size_t ind) const;

    reference operator*();

    pointer operator->() const;

    bool operator<(const DequeIterator& rhs) const;

    bool operator>(const DequeIterator& rhs) const;
    bool operator<=(const DequeIterator& rhs) const;
    bool operator>=(const DequeIterator& rhs) const;
    bool operator==(const DequeIterator& rhs) const;

    bool operator!=(const DequeIterator& rhs) const;

    difference_type operator-(const DequeIterator& rhs) const;

    size_t segment;
    size_t index;
    Deque<T>* deque;
  };

  reverse_iterator rbegin() noexcept;
  const_reverse_iterator rbegin() const noexcept;
  reverse_iterator rend() noexcept;
  const_reverse_iterator rend() const noexcept;
  const_reverse_iterator crbegin() const noexcept;
  const_reverse_iterator crend() const noexcept;

  iterator begin();
  const_iterator begin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  void insert(iterator ind, const T& value);

  void erase(iterator ind);

  ~Deque();

  template <typename... Args>
  void emplace_back(Args&&... args);

 private:
  static constexpr size_t kSegmentSize = 80;
  std::vector<T*> data_;
  size_t kBnum_ = 1;
  static const size_t kSize = 64;
  DequeIterator<false> start_;
  DequeIterator<false> finish_;
};

template <typename T>
Deque<T>::Deque() : data_(1), start_(0, 0, this), finish_(0, 0, this) {}

template <typename T>
Deque<T>::Deque(const Deque& other) : Deque() {
  for (size_t i = 0; i < other.size(); ++i) {
    push_back(other[i]);
  }
}

template <typename T>
Deque<T>::Deque(size_t count, const T& value)
    : start_(0, 0, this), finish_(count / kSize, count % kSize, this) {
  size_t needed_blocks = (count + kSize - 1) / kSize;
  data_.resize(needed_blocks);
  try {
    for (auto& segment : data_) {
      segment = reinterpret_cast<T*>(new char8_t[sizeof(T) * kSize]);
      for (size_t i = 0; i < kSize; i++) {
        new (segment + i) T(value);
      }
    }
  } catch (...) {
    for (auto& segment : data_) {
      if (segment != nullptr) {
        for (size_t i = 0; i < kSize; ++i) {
          segment[i].~T();
        }
        operator delete[](segment);
      }
    }
    throw;
  }
}

template <typename T>
void Deque<T>::add_one(const T& value) {
  start_ = iterator(0, 0, this);
  finish_ = iterator(1 / kSize, 1 % kSize, this);
  size_t needed_blocks = (1 + kSize - 1) / kSize;
  data_.resize(needed_blocks);
  for (auto& segment : data_) {
    segment = reinterpret_cast<T*>(new char8_t[sizeof(T) * kSize]);
  }
  data_[0][0] = value;
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& rhs) {
  if (this == &rhs) {
    return *this;
  }

  clear();

  for (size_t i = 0; i < rhs.size(); ++i) {
    push_back(rhs[i]);
  }

  return *this;
}

template <typename T>
void Deque<T>::clear() {
  for (auto& segment : data_) {
    if (segment != nullptr) {
      for (size_t i = 0; i < kSize; ++i) {
        segment[i].~T();
      }
      delete[] reinterpret_cast<char*>(segment);
    }
  }
  data_.clear();
  start_ = iterator(0, 0, this);
  finish_ = iterator(0, 0, this);
}

template <typename T>
size_t Deque<T>::size() const {
  return (finish_.segment - start_.segment) * kSize + finish_.index -
         start_.index;
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
  size_t adjusted_index = start_.index + index;
  size_t segment = start_.segment + adjusted_index / kSize;
  size_t index_within_segment = adjusted_index % kSize;
  return data_[segment][index_within_segment];
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
  size_t adjusted_index = start_.index + index;
  size_t segment = start_.segment + adjusted_index / kSize;
  size_t index_within_segment = adjusted_index % kSize;
  return data_[segment][index_within_segment];
}

template <typename T>
T& Deque<T>::at(size_t index) {
  if (index >= size()) {
    throw std::out_of_range("index out of range");
  }
  return operator[](index);
}

template <typename T>
const T& Deque<T>::at(size_t index) const {
  if (index >= size()) {
    throw std::out_of_range("index out of range");
  }
  return operator[](index);
}

template <typename T>
void Deque<T>::push_back(const T& value) {
  if (empty()) {
    add_one(value);
    return;
  }
  if (finish_.index == kSize) {
    if (finish_.segment == data_.size() - 1) {
      data_.emplace_back(reinterpret_cast<T*>(new char8_t[sizeof(T) * kSize]));
    }
    ++finish_.segment;
    finish_.index = 0;
  }

  new (data_[finish_.segment] + finish_.index) T(value);

  finish_.index++;

  if (finish_.index == kSize) {
    if (finish_.segment == data_.size() - 1) {
      data_.emplace_back(reinterpret_cast<T*>(new char8_t[sizeof(T) * kSize]));
    }
    ++finish_.segment;
    finish_.index = 0;
  }
}

template <typename T>
void Deque<T>::push_front(const T& value) {
  if (start_.index == 0) {
    if (start_.segment == 0) {
      data_.insert(data_.end(),
                   reinterpret_cast<T*>(new char8_t[sizeof(T) * kSize]));
      ++finish_.segment;
      start_.segment = 1;
    }
    --start_.segment;
    start_.index = kSize - 1;
  } else {
    --start_.index;
  }
  new (data_[start_.segment] + start_.index) T(value);
}

template <typename T>
void Deque<T>::pop_back() {
  if (empty()) {
    throw std::out_of_range("deque is empty");
  }
  if (finish_.index == 0 && finish_.segment > 0) {
    --finish_.segment;
    finish_.index = kSize;
  }
  (data_[finish_.segment] + --finish_.index)->~T();
}

template <typename T>
void Deque<T>::pop_front() {
  if (empty()) {
    throw std::out_of_range("deque is empty");
  }
  (data_[start_.segment] + start_.index++)->~T();
  if (start_.index == kSize && start_.segment + 1 < data_.size()) {
    ++start_.segment;
    start_.index = 0;
  }
}

template <typename T>
bool Deque<T>::empty() const {
  return size() == 0;
};

template <typename T>
void Deque<T>::insert(iterator ind, const T& value) {
  if (empty()) {
    push_back(value);
    return;
  }
  size_t index = ind - begin();
  push_back(data_[finish_.segment][finish_.index]);
  for (size_t i = size() - 1; i > index; --i) {
    std::swap(operator[](i - 1), operator[](i));
  }
  operator[](index) = value;
}

template <typename T>
void Deque<T>::erase(iterator ind) {
  size_t index = ind - begin();
  for (size_t i = index; i < size() - 1; ++i) {
    std::swap(operator[](i), operator[](i + 1));
  }
  pop_back();
}

template <typename T>
Deque<T>::~Deque() {
  for (auto& segment : data_) {
    if (segment != nullptr) {
      for (size_t i = 0; i < kSize; ++i) {
        segment[i].~T();
      }
      delete[] reinterpret_cast<char*>(segment);
    }
  }
}

template <typename T>
template <typename... Args>
void Deque<T>::emplace_back(Args&&... args) {
  if (!data_.empty()) {
    size_t num_segments = kBnum_ * kSegmentSize;
    data_.reserve(num_segments);
    for (size_t i = 0; i < num_segments; ++i) {
      data_[i] = new T[kSegmentSize];
    }
  }
  auto cur = finish_;
  new (data_[cur.segment] + cur.index) T(std::forward<Args>(args)...);
  ++finish_;
}

template <typename T>
template <bool IsConst>
Deque<T>::DequeIterator<IsConst>::operator DequeIterator<true>() const {
  return DequeIterator<true>(segment, index);
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>::reference
Deque<T>::DequeIterator<IsConst>::operator*() {
  return deque->data_[segment][index];
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>::pointer
Deque<T>::DequeIterator<IsConst>::operator->() const {
  return &deque->data_[segment][index];
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>&
Deque<T>::DequeIterator<IsConst>::operator++() {
  index++;
  if (index == deque->kSize) {
    index = 0;
    segment++;
  }
  return *this;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>&
Deque<T>::DequeIterator<IsConst>::operator--() {
  if (index == 0) {
    index = deque->kSize;
    segment--;
  }
  index--;
  return *this;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>
Deque<T>::DequeIterator<IsConst>::operator++(int) {
  DequeIterator res = *this;
  operator++();
  return res;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>
Deque<T>::DequeIterator<IsConst>::operator--(int) {
  DequeIterator res = *this;
  operator--();
  return res;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>&
Deque<T>::DequeIterator<IsConst>::operator+=(size_t ind) {
  segment = (segment * kSize + index + ind) / kSize;
  index = (index + ind) % kSize;
  return *this;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>&
Deque<T>::DequeIterator<IsConst>::operator-=(size_t ind) {
  return operator+=(-ind);
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>
Deque<T>::DequeIterator<IsConst>::operator+(size_t ind) const {
  DequeIterator res = *this;
  res += ind;
  return res;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>
Deque<T>::DequeIterator<IsConst>::operator-(size_t ind) const {
  DequeIterator res = *this;
  res -= ind;
  return res;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::DequeIterator<IsConst>::operator<(
    const Deque<T>::template DequeIterator<IsConst>& rhs) const {
  return segment < rhs.segment || (segment == rhs.segment && index < rhs.index);
}

template <typename T>
template <bool IsConst>
bool Deque<T>::DequeIterator<IsConst>::operator>(
    const Deque<T>::template DequeIterator<IsConst>& rhs) const {
  return rhs < *this;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::DequeIterator<IsConst>::operator<=(
    const Deque<T>::template DequeIterator<IsConst>& rhs) const {
  return !(rhs < *this);
}

template <typename T>
template <bool IsConst>
bool Deque<T>::DequeIterator<IsConst>::operator>=(
    const Deque<T>::template DequeIterator<IsConst>& rhs) const {
  return !(*this < rhs);
}

template <typename T>
template <bool IsConst>
bool Deque<T>::DequeIterator<IsConst>::operator==(
    const Deque<T>::template DequeIterator<IsConst>& rhs) const {
  return segment == rhs.segment && index == rhs.index;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::DequeIterator<IsConst>::operator!=(
    const Deque<T>::template DequeIterator<IsConst>& rhs) const {
  return !(*this == rhs);
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template DequeIterator<IsConst>::difference_type
Deque<T>::DequeIterator<IsConst>::operator-(const DequeIterator& rhs) const {
  return difference_type(segment - rhs.segment) * deque->kSize + index -
         rhs.index;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
  return iterator(start_.segment, start_.index, this);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const noexcept {
  return start_;
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() noexcept {
  return finish_;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const noexcept {
  return finish_;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const noexcept {
  return start_;
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const noexcept {
  return finish_;
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() noexcept {
  return reverse_iterator(end());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const noexcept {
  return const_reverse_iterator(end());
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() noexcept {
  return reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const noexcept {
  return const_reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const noexcept {
  return const_reverse_iterator(cend());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const noexcept {
  return const_reverse_iterator(cbegin());
}