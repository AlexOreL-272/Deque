#include <exception>
#include <type_traits>
#include <stdexcept>
#include <vector>

template<typename T>
class Deque {
 public:
  std::vector<T*> outer_;
 private:

  size_t num_of_arrays_above_ = 1;
  size_t num_of_arrays_beyond_ = 1;
  size_t actual_size_ = 0;

  T** begin_vector_;
  size_t begin_pos_;

  T** end_vector_;
  size_t end_pos_;

  static const size_t kInnerArraySize = 1000;

 public:
  template <bool IsConst, bool IsReversed>
  class Iterator;

  using const_iterator = Iterator<true, false>;
  using iterator = Iterator<false, false>;
  using reverse_iterator = Iterator<false, true>;
  using const_reverse_iterator = Iterator<true, true>;

  Deque() : outer_(3, nullptr), begin_vector_(&outer_[1]), begin_pos_(0), end_vector_(&outer_[1]), end_pos_(0) {
    outer_[1] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
  }

  explicit Deque(const size_t& count) : actual_size_(count) {
    auto uses_whole_number = static_cast<size_t>(count % kInnerArraySize == 0);

    size_t num_of_arrays = count / kInnerArraySize + uses_whole_number + 1;
    outer_.resize(num_of_arrays * 3, nullptr);

    for (size_t i = num_of_arrays; i < 2 * num_of_arrays - uses_whole_number; i++) {
      outer_[i] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
      for (size_t j = 0; j < kInnerArraySize; j++) {
        outer_[i][j] = T{};
      }
    }

    num_of_arrays_above_ = num_of_arrays;
    num_of_arrays_beyond_ = num_of_arrays;

    begin_vector_ = &outer_[num_of_arrays];
    begin_pos_ = 0;

    end_vector_ = &outer_[2 * num_of_arrays - 1 - uses_whole_number];
    end_pos_ = count % kInnerArraySize;

    for (size_t i = 0; i < end_pos_; i++) {
      (*end_vector_)[i] = T{};
    }
  }

  Deque(const size_t& count, const T& value) : actual_size_(count) {
    auto uses_whole_number = static_cast<size_t>(count % kInnerArraySize == 0);

    size_t num_of_arrays = count / kInnerArraySize + uses_whole_number + 1;
    outer_.resize(num_of_arrays * 3, nullptr);

    num_of_arrays_above_ = num_of_arrays;
    num_of_arrays_beyond_ = num_of_arrays;

    // outer_[num_of_arrays - 1] = new T[kInnerArraySize];
    for (size_t i = num_of_arrays; i < 2 * num_of_arrays - 1 - uses_whole_number; i++) {
      outer_[i] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
      for (size_t j = 0; j < kInnerArraySize; j++) {
        outer_[i][j] = value;
      }
    }

    outer_[2 * num_of_arrays - 1 - uses_whole_number] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);

    end_vector_ = &outer_[2 * num_of_arrays - 1 - uses_whole_number];
    end_pos_ = count % kInnerArraySize;

    begin_vector_ = &outer_[num_of_arrays];
    begin_pos_ = 0;

    for (size_t i = 0; i < end_pos_; i++) {
      (*end_vector_)[i] = value;
    }
  }

  Deque(const Deque& obj_to_copy) {
    for (int i = 0; i < outer_.size(); i++) {
      delete[] outer_[i];
    }

    outer_.resize(obj_to_copy.outer_.size(), nullptr);

    num_of_arrays_above_ = obj_to_copy.num_of_arrays_above_;
    num_of_arrays_beyond_ = obj_to_copy.num_of_arrays_beyond_;
    actual_size_ = obj_to_copy.actual_size_;

    for (size_t i = num_of_arrays_above_; i < outer_.size() - num_of_arrays_beyond_ - 1; i++) {
      outer_[i] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);;
      for (size_t j = 0; j < kInnerArraySize; j++) {
        outer_[i][j] = obj_to_copy.outer_[i][j];
      }
    }

    begin_vector_ = &outer_[num_of_arrays_above_];
    begin_pos_ = obj_to_copy.begin_pos_;

    outer_[outer_.size() - num_of_arrays_beyond_ - 1] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);;;
    end_vector_ = end_vector_ = &outer_[outer_.size() - num_of_arrays_beyond_ - 1];
    end_pos_ = obj_to_copy.end_pos_;

    for (size_t i = 0; i < end_pos_; i++) {
      (*end_vector_)[i] = (*obj_to_copy.end_vector_)[i];
    }
  }

  ~Deque() {
    for (int i = 0; i < outer_.size(); i++) {
      delete[] outer_[i];
    }
  }

  Deque& operator=(const Deque& obj_to_assign) {
    Deque copy(obj_to_assign);

    for (int i = 0; i < outer_.size(); i++) {
      delete[] outer_[i];
    }

    outer_.resize(copy.outer_.size(), nullptr);

    num_of_arrays_above_ = copy.num_of_arrays_above_;
    num_of_arrays_beyond_ = copy.num_of_arrays_beyond_;
    actual_size_ = copy.actual_size_;

    for (size_t i = num_of_arrays_above_; i < outer_.size() - num_of_arrays_beyond_ - 1; i++) {
      outer_[i] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);;
      for (size_t j = 0; j < kInnerArraySize; j++) {
        outer_[i][j] = copy.outer_[i][j];
      }
    }

    begin_vector_ = &outer_[num_of_arrays_above_];
    begin_pos_ = copy.begin_pos_;

    outer_[outer_.size() - num_of_arrays_beyond_ - 1] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);;
    end_vector_ = end_vector_ = &outer_[outer_.size() - num_of_arrays_beyond_ - 1];
    end_pos_ = copy.end_pos_;

    for (size_t i = 0; i < end_pos_; i++) {
      (*end_vector_)[i] = (*copy.end_vector_)[i];
    }

    return *this;
  }

  size_t size() const noexcept { return actual_size_; }

  bool empty() const noexcept { return actual_size_ == 0; }

  std::vector<T*> GetOuter() const { return outer_; }

  T& operator[](size_t pos) {
    size_t num_of_arrays_to_skip = pos / kInnerArraySize;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  const T& operator[](size_t pos) const {
    size_t num_of_arrays_to_skip = pos / kInnerArraySize;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  T& at(size_t pos) {
    if (pos < 0 || pos >= actual_size_) {
      throw std::out_of_range("You are dingus!\n");
    }

    size_t num_of_arrays_to_skip = pos / kInnerArraySize;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  const T& at(size_t pos) const {
    if (pos < 0 || pos >= actual_size_) {
      throw std::out_of_range("You are dingus!\n");
    }

    size_t num_of_arrays_to_skip = pos / kInnerArraySize;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  void resize_if_needed(bool necessary = false) {
    size_t cur_size = outer_.size();

    if ((end_vector_ != &outer_[cur_size - 1] + 1 || end_pos_ != 0) &&
        (begin_vector_ != &outer_[0] || begin_pos_ != 0) && !necessary) {
      return;
    }

    std::vector<T*> new_vec(cur_size * 3, nullptr);
    for (size_t i = cur_size - 1; i < 2 * cur_size - 1; i++) {
      new_vec[i] = outer_[i - cur_size + 1];
    }
    new_vec[2 * cur_size - 1] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);


    num_of_arrays_above_ += cur_size;
    num_of_arrays_beyond_ += cur_size;

    auto addr = &outer_[0];

    outer_ = new_vec;

    begin_vector_ = &outer_[cur_size - 1] + (begin_vector_ - addr);
    end_vector_ = &outer_[cur_size - 1] + (end_vector_ - addr);
  }

  void push_back(const T& elem) {
    resize_if_needed();

    (*end_vector_)[end_pos_++] = elem;
    actual_size_++;

    if (end_pos_ == kInnerArraySize) {
      end_pos_ = 0;
      end_vector_++;
      num_of_arrays_beyond_--;

      if (*end_vector_ == nullptr) {
        *end_vector_ = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
      }
    }
  }

  void push_front(const T& elem) {
    resize_if_needed();

    if (begin_pos_ == 0) {
      begin_pos_ = kInnerArraySize;
      begin_vector_--;
      num_of_arrays_above_--;

      if (*begin_vector_ == nullptr) {
        *begin_vector_ = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
      }
    }

    (*begin_vector_)[--begin_pos_] = elem;
    actual_size_++;
  }

  void pop_back() {
    if (actual_size_ == 0) {
      return;
    }

    if (end_pos_ == 0) {
      end_pos_ = kInnerArraySize - 1;
      end_vector_--;
      num_of_arrays_beyond_++;
    }
    actual_size_--;
  }

  void pop_front() {
    if (actual_size_ == 0) {
      return;
    }

    if (begin_pos_ == kInnerArraySize - 1) {
      begin_pos_ = 0;
      begin_vector_++;
      num_of_arrays_above_++;
    }

    actual_size_--;
  }

  iterator begin() {
    return {begin_vector_, begin_pos_};
  }

  iterator end() {
    return {end_vector_, end_pos_};
  }

  const_iterator begin() const {
    return {begin_vector_, begin_pos_};
  }

  const_iterator end() const {
    return {end_vector_, end_pos_};
  }

  const_iterator cbegin() const {
    return {begin_vector_, begin_pos_};
  }

  const_iterator cend() const {
    return {end_vector_, end_pos_};
  }

  reverse_iterator rbegin() {
    if (end_pos_ == 0) {
      return {end_vector_ - 1, kInnerArraySize - 1};
    }
    return {end_vector_, end_pos_ - 1};
  }

  reverse_iterator rend() {
    if (begin_pos_ == 0) {
      return {begin_vector_ - 1, kInnerArraySize - 1};
    }
    return {begin_vector_, begin_pos_ - 1};
  }

  const_reverse_iterator rbegin() const {
    if (end_pos_ == 0) {
      return {end_vector_ - 1, kInnerArraySize - 1};
    }
    return {end_vector_, end_pos_ - 1};
  }

  const_reverse_iterator rend() const {
    if (begin_pos_ == 0) {
      return {begin_vector_ - 1, kInnerArraySize - 1};
    }
    return {begin_vector_, begin_pos_ - 1};
  }

  const_reverse_iterator crbegin() const {
    if (end_pos_ == 0) {
      return {end_vector_ - 1, kInnerArraySize - 1};
    }
    return {end_vector_, end_pos_ - 1};
  }

  const_reverse_iterator crend() const {
    if (begin_pos_ == 0) {
      return {begin_vector_ - 1, kInnerArraySize - 1};
    }
    return {begin_vector_, begin_pos_ - 1};
  }

  void insert(iterator it, const T& val) {
    auto end_iter = end();

    T tmp1(val), tmp2(val);
    while(it < end_iter) {
      if (it.GetPtr() == nullptr) {
        resize_if_needed(true);
      }

      tmp2 = *it;
      *it = tmp1;
      tmp1 = tmp2;
      it++;
    }

    push_back(tmp1);
  }

  void erase(iterator it) {
    auto end_iter = end();

    while(it <= end_iter) {
      *it = *(it + 1);
      it++;
    }

    actual_size_--;

    if (end_pos_ == 0) {
      end_pos_ = kInnerArraySize - 1;
      end_vector_--;
    } else {
      end_pos_--;
    }
  }
};

template <typename T>
template <bool IsConst, bool IsReversed>
class Deque<T>::Iterator {
 private:
  T** ptr_;
  size_t pos_;

 public:
  using is_const = std::conditional_t<IsConst, const T, T>;
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::remove_cv_t<T>;
  using difference_type = std::ptrdiff_t;
  using pointer = is_const*;
  using reference = is_const&;

  Iterator(): ptr_(nullptr), pos_(0) {}

  Iterator(T** ptr, const size_t& pos): ptr_(ptr), pos_(pos) {}

  Iterator(const Iterator<IsConst, IsReversed>& other): ptr_(other.ptr_), pos_(other.pos_) {}

  /*explicit*/ operator Iterator<true, IsReversed>() const {
    return {ptr_, pos_};
  }

  /*explicit*/ operator Iterator<false, IsReversed>() const {
    if (IsConst) {
      throw std::bad_cast();
    }
    return {ptr_, pos_};
  }

  Iterator operator++(int) {
    Iterator copy(*this);
    ++*this;
    return copy;
  }

  Iterator& operator++() {
    if (!IsReversed) {
      if (pos_ == kInnerArraySize - 1) {
        pos_ = 0;
        ptr_++;
      } else {
        pos_++;
      }
    } else {
      if (pos_ == 0) {
        pos_ = kInnerArraySize - 1;
        ptr_--;
      } else {
        pos_--;
      }
    }
    return *this;
  }

  Iterator operator--(int) {
    Iterator copy(*this);
    --*this;
    return copy;
  }

  Iterator& operator--() {
    if (!IsReversed) {
      if (pos_ == 0) {
        pos_ = kInnerArraySize - 1;
        ptr_--;
      } else {
        pos_--;
      }
    } else {
      if (pos_ == kInnerArraySize - 1) {
        pos_ = 0;
        ptr_++;
      } else {
        pos_++;
      }
    }
    return *this;
  }

  Iterator& operator+=(int num) {
    if (num < 0) {
      *this -= (-num);
      return *this;
    }

    if (!IsReversed) {
      ptr_ += (pos_ + num) / kInnerArraySize;
      pos_ = (pos_ + num) % kInnerArraySize;
    } else {
      if (static_cast<int>(pos_ - num) < 0) {
        size_t num_of_layers_to_skip = (num - pos_) / kInnerArraySize;
        size_t new_pos = (num % kInnerArraySize) - pos_ ;
        pos_ = kInnerArraySize - new_pos;
        ptr_ -= (num_of_layers_to_skip + 1);
      } else {
        pos_ -= num;
      }
    }

    return *this;
  }

  Iterator operator+(int num) const {
    Iterator copy(ptr_, pos_);
    copy += num;
    return copy;
  }

  Iterator& operator-=(int num) {
    if (num < 0) {
      *this += (-num);
      return *this;
    }

    if (!IsReversed) {
      if (static_cast<int>(pos_) - num < 0) {
        size_t num_of_layers_to_skip = (num - pos_ - 1) / kInnerArraySize;
        size_t new_pos = (num % kInnerArraySize) - pos_;
        pos_ = (kInnerArraySize - new_pos) % kInnerArraySize;
        ptr_ -= (num_of_layers_to_skip + 1);
      } else {
        pos_ -= num;
      }
    } else {
      ptr_ += (pos_ + num) / kInnerArraySize;
      pos_ = (pos_ + num) % kInnerArraySize;
    }

    return *this;
  }

  Iterator operator-(int num) const {
    Iterator copy(ptr_, pos_);
    copy -= num;
    return copy;
  }

  friend Iterator operator+(int num, Iterator it) {
    return it + num;
  }

  is_const& operator[](int num) const {
    return *(*this + num);
  }

  T** GetPtr() const { return ptr_; }

  size_t GetPos() const { return pos_; }

  template <bool ConstR>
  short compareTo(const Iterator<ConstR, IsReversed> other) const {
    if (ptr_ == other.GetPtr()) {
      return pos_ == (other.GetPos() ? 0 : pos_ < other.GetPos() ? -1 : 1) * (1 - 2 * static_cast<short>(IsReversed));
    }
    return (ptr_ < other.GetPtr() ? -1 : 1) * (1 - 2 * static_cast<short>(IsReversed));
  }

  template <bool ConstR>
  bool operator<(const Iterator<ConstR, IsReversed> other) const {
    return compareTo(other) == -1;
  }

  template <bool ConstR>
  bool operator>(const Iterator<ConstR, IsReversed> other) const {
    return compareTo(other) == 1;
  }

  template <bool ConstR>
  bool operator<=(const Iterator<ConstR, IsReversed> other) const {
    return compareTo(other) != 1;
  }

  template <bool ConstR>
  bool operator>=(const Iterator<ConstR, IsReversed> other) const {
    return compareTo(other) != -1;
  }

  template <bool ConstR>
  bool operator==(const Iterator<ConstR, IsReversed> other) const {
    return compareTo(other) == 0;
  }

  template <bool ConstR>
  bool operator!=(const Iterator<ConstR, IsReversed> other) const {
    return compareTo(other) != 0;
  }

  template <bool ConstR>
  int operator-(const Iterator<ConstR, IsReversed> other) const {
    return (ptr_ - other.ptr_) * kInnerArraySize + pos_ - other.pos_;
  }

  reference operator*() {
    return (*ptr_)[pos_];
  }

  pointer operator->() {
    return *ptr_ + pos_;
  }
};
