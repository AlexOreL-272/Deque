#include <exception>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T>
class Deque {
 private:
  std::vector<T*> outer_;

  size_t num_of_arrays_above_ = 1;
  size_t num_of_arrays_beyond_ = 1;
  size_t actual_size_ = 0;

  T** begin_vector_;
  size_t begin_pos_;

  T** end_vector_;
  size_t end_pos_;

  static const size_t kInnerArraySize = 1000;

 public:
  // std::vector<T*> outer_;

  template <bool IsConst, bool IsReversed>
  class Iterator;

  using const_iterator = Iterator<true, false>;
  using iterator = Iterator<false, false>;
  using reverse_iterator = Iterator<false, true>;
  using const_reverse_iterator = Iterator<true, true>;

  Deque() : begin_pos_(0), end_pos_(0) {
    outer_.resize(3, nullptr);
    outer_[1] = reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
    begin_vector_ = &outer_[1];
    end_vector_ = &outer_[1];
  }

  explicit Deque(const size_t& count) : actual_size_(count) {
    auto uses_whole_number = static_cast<size_t>(count % kInnerArraySize == 0);

    size_t num_of_arrays = count / kInnerArraySize + uses_whole_number + 1;
    outer_.resize(num_of_arrays * 3, nullptr);

    for (size_t i = num_of_arrays; i < 2 * num_of_arrays - uses_whole_number;
         i++) {
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
    size_t num_of_arrays;

    try {
      auto uses_whole_number =
          static_cast<size_t>(count % kInnerArraySize == 0);

      num_of_arrays = count / kInnerArraySize + uses_whole_number + 1;
      outer_.resize(num_of_arrays * 3, nullptr);

      num_of_arrays_above_ = num_of_arrays;
      num_of_arrays_beyond_ = num_of_arrays;

      // outer_[num_of_arrays - 1] = new T[kInnerArraySize];
      for (size_t i = num_of_arrays;
           i < 2 * num_of_arrays - 1 - uses_whole_number; i++) {
        outer_[i] =
            reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);

        // std::fill_n(outer_[i], kInnerArraySize * sizeof(T), value);
        for (size_t j = 0; j < kInnerArraySize; j++) {
          // outer_[i][j] = value;
          new (outer_[i] + j) T(value);
        }
      }

      outer_[2 * num_of_arrays - 1 - uses_whole_number] =
          reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);

      end_vector_ = &outer_[2 * num_of_arrays - 1 - uses_whole_number];
      end_pos_ = count % kInnerArraySize;

      begin_vector_ = &outer_[num_of_arrays];
      begin_pos_ = 0;

      for (size_t i = 0; i < end_pos_; i++) {
        // (*end_vector_)[i] = value;
        new (*end_vector_ + i) T(value);
      }
    } catch (...) {
      for (size_t i = 0; i < num_of_arrays * 3; i++) {
        delete[] outer_[i];
      }
      throw -1;  // std::runtime_error("Could not constuct an object\n");
    }
  }

  Deque(const Deque& obj_to_copy) {
    size_t size;

    try {
      for (size_t i = 0; i < outer_.size(); i++) {
        delete[] outer_[i];
      }

      size = obj_to_copy.outer_.size();
      outer_.resize(size, nullptr);

      num_of_arrays_above_ = obj_to_copy.num_of_arrays_above_;
      num_of_arrays_beyond_ = obj_to_copy.num_of_arrays_beyond_;
      actual_size_ = obj_to_copy.actual_size_;

      for (size_t i = num_of_arrays_above_;
           i < size - num_of_arrays_beyond_ - 1; i++) {
        outer_[i] =
            reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);

        for (size_t j = 0; j < kInnerArraySize; j++) {
          outer_[i][j] = obj_to_copy.outer_[i][j];
        }
      }

      begin_vector_ = &outer_[num_of_arrays_above_];
      begin_pos_ = obj_to_copy.begin_pos_;

      outer_[size - num_of_arrays_beyond_ - 1] =
          reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);

      end_vector_ = &outer_[size - num_of_arrays_beyond_ - 1];
      end_pos_ = obj_to_copy.end_pos_;

      if (*obj_to_copy.end_vector_ != nullptr) {
        for (size_t i = 0; i < end_pos_; i++) {
          (*end_vector_)[i] = (*obj_to_copy.end_vector_)[i];
        }
      }
    } catch (...) {
      for (size_t i = 0; i < size; i++) {
        delete[] outer_[i];
      }
      throw -1;  // std::runtime_error("Could not copy-constuct an object\n");
    }
  }

  ~Deque() {
    for (size_t i = 0; i < outer_.size(); i++) {
      delete[] outer_[i];
    }
  }

  Deque& operator=(const Deque& obj_to_assign) {
    Deque copy(obj_to_assign);  // but it might throw an exception there...

    try {
      size_t size;

      for (size_t i = 0; i < outer_.size(); i++) {
        delete[] outer_[i];
      }

      size = copy.outer_.size();
      outer_.resize(size, nullptr);

      num_of_arrays_above_ = copy.num_of_arrays_above_;
      num_of_arrays_beyond_ = copy.num_of_arrays_beyond_;
      actual_size_ = copy.actual_size_;

      for (size_t i = num_of_arrays_above_;
           i < size - num_of_arrays_beyond_ - 1; i++) {
        outer_[i] =
            reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
        for (size_t j = 0; j < kInnerArraySize; j++) {
          outer_[i][j] = copy.outer_[i][j];
        }
      }

      begin_vector_ = &outer_[num_of_arrays_above_];
      begin_pos_ = copy.begin_pos_;

      outer_[size - num_of_arrays_beyond_ - 1] =
          reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
      end_vector_ = end_vector_ = &outer_[size - num_of_arrays_beyond_ - 1];
      end_pos_ = copy.end_pos_;

      for (size_t i = 0; i < end_pos_; i++) {
        (*end_vector_)[i] = (*copy.end_vector_)[i];
      }

      return *this;
    } catch (...) {
      *this = copy;
      throw -1;  // std::runtime_error("Could not copy-constuct an object\n");
    }
  }

  size_t size() const noexcept { return actual_size_; }

  bool empty() const noexcept { return actual_size_ == 0; }

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
    if (pos >= actual_size_) {
      throw std::out_of_range("You are dingus!\n");
    }

    size_t num_of_arrays_to_skip = pos / kInnerArraySize;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  const T& at(size_t pos) const {
    if (pos >= actual_size_) {
      throw std::out_of_range("You are dingus!\n");
    }

    size_t num_of_arrays_to_skip = pos / kInnerArraySize;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  void resize_if_needed() {
    size_t cur_size = outer_.size();

    if ((end_vector_ < &outer_[cur_size - 1] + 1 || end_pos_ != 0) &&
        (begin_vector_ > (outer_).data() || begin_pos_ != 0)) {
      return;
    }

    std::vector<T*> new_vec(cur_size * 3, nullptr);
    for (size_t i = cur_size - 1; i < 2 * cur_size - 1; i++) {
      new_vec[i] = outer_[i - cur_size + 1];
    }
    new_vec[2 * cur_size - 1] =
        reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);

    num_of_arrays_above_ += cur_size - 1;
    num_of_arrays_beyond_ += cur_size + 1;

    auto addr = (outer_).data();

    outer_ = new_vec;

    begin_vector_ = &outer_[cur_size - 1] + (begin_vector_ - addr);
    end_vector_ = &outer_[cur_size - 1] + (end_vector_ - addr);
  }

  void push_back(const T& elem) {
    resize_if_needed();

    if (*end_vector_ == nullptr) {
      *end_vector_ =
          reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
    }

    try {
      //(*end_vector_)[end_pos_++] = elem;
      new (*end_vector_ + end_pos_) T(elem);
      end_pos_++;
    } catch (...) {
      end_pos_--;
      throw -2;  // std::runtime_error("Could not push back this value\n");
    }

    actual_size_++;

    if (end_pos_ == kInnerArraySize) {
      end_pos_ = 0;
      end_vector_++;
      num_of_arrays_beyond_--;
    }

    resize_if_needed();
  }

  void push_front(const T& elem) {
    resize_if_needed();

    size_t copy_pos = begin_pos_;
    T** copy_begin = begin_vector_;

    if (begin_pos_ == 0) {
      begin_pos_ = kInnerArraySize;
      begin_vector_--;
      num_of_arrays_above_--;

      if (*begin_vector_ == nullptr) {
        *begin_vector_ =
            reinterpret_cast<T*>(new int8_t[kInnerArraySize * sizeof(T)]);
      }
    }

    try {
      // (*begin_vector_)[--begin_pos_] = elem;
      begin_pos_--;
      new (*begin_vector_ + begin_pos_) T(elem);
    } catch (...) {
      begin_pos_ = copy_pos;
      begin_vector_ = copy_begin;
      throw -2;  // std::runtime_error("Could not copy-constuct an object\n");
    }
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

  iterator begin() { return iterator(begin_vector_, begin_pos_); }

  iterator end() { return iterator(end_vector_, end_pos_); }

  const_iterator begin() const {
    return const_iterator(begin_vector_, begin_pos_);
  }

  const_iterator end() const { return const_iterator(end_vector_, end_pos_); }

  const_iterator cbegin() const {
    return const_iterator(begin_vector_, begin_pos_);
  }

  const_iterator cend() const { return const_iterator(end_vector_, end_pos_); }

  reverse_iterator rbegin() {
    if (end_pos_ == 0) {
      return reverse_iterator(end_vector_ - 1, kInnerArraySize - 1);
    }
    return reverse_iterator(end_vector_, end_pos_ - 1);
  }

  reverse_iterator rend() {
    if (begin_pos_ == 0) {
      return reverse_iterator(begin_vector_ - 1, kInnerArraySize - 1);
    }
    return reverse_iterator(begin_vector_, begin_pos_ - 1);
  }

  const_reverse_iterator rbegin() const {
    if (end_pos_ == 0) {
      return const_reverse_iterator(end_vector_ - 1, kInnerArraySize - 1);
    }
    return const_reverse_iterator(end_vector_, end_pos_ - 1);
  }

  const_reverse_iterator rend() const {
    if (begin_pos_ == 0) {
      return const_reverse_iterator(begin_vector_ - 1, kInnerArraySize - 1);
    }
    return const_reverse_iterator(begin_vector_, begin_pos_ - 1);
  }

  const_reverse_iterator crbegin() const {
    if (end_pos_ == 0) {
      return const_reverse_iterator(end_vector_ - 1, kInnerArraySize - 1);
    }
    return const_reverse_iterator(end_vector_, end_pos_ - 1);
  }

  const_reverse_iterator crend() const {
    if (begin_pos_ == 0) {
      return const_reverse_iterator(begin_vector_ - 1, kInnerArraySize - 1);
    }
    return const_reverse_iterator(begin_vector_, begin_pos_ - 1);
  }

  void insert(iterator iter, const T& val) {
    Deque copy(*this);

    try {
      auto end_iter = end();

      T tmp1(val);
      T tmp2(val);
      while (iter < end_iter) {
        resize_if_needed();

        tmp2 = *iter;
        *iter = tmp1;
        tmp1 = tmp2;
        iter++;
      }

      push_back(tmp1);
    } catch (...) {
      *this = copy;
      throw -3;  // std::runtime_error("Could not insert a value\n");
    }
  }

  void erase(iterator iter) {
    Deque copy(*this);

    try {
      auto end_iter = end();

      while (iter <= end_iter) {
        *iter = *(iter + 1);
        iter++;
      }
    } catch (...) {
      *this = copy;
      throw -3;  // std::runtime_error("Could not erase the value\n");
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

  Iterator() : ptr_(nullptr), pos_(0) {}

  Iterator(T** ptr, const size_t& pos) : ptr_(ptr), pos_(pos) {}

  // Iterator(const Iterator<IsConst, IsReversed>& other)
  //     : ptr_(other.ptr_), pos_(other.pos_) {}

  operator Iterator<true, IsReversed>() const { return {ptr_, pos_}; }

  operator Iterator<false, IsReversed>() const {
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
        size_t new_pos = (num % kInnerArraySize) - pos_;
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

  T** get_ptr() const { return ptr_; }

  size_t get_pos() const { return pos_; }

  template <bool ConstR>
  short compare_to(const Iterator<ConstR, IsReversed> kOther) const {
    if (ptr_ == kOther.get_ptr()) {
      return (pos_ == kOther.get_pos()  ? 0
              : pos_ < kOther.get_pos() ? -1
                                        : 1) *
             (1 - 2 * static_cast<short>(IsReversed));
    }
    return (ptr_ < kOther.get_ptr() ? -1 : 1) *
           (1 - 2 * static_cast<short>(IsReversed));
  }

  template <bool ConstR>
  bool operator<(const Iterator<ConstR, IsReversed> kOther) const {
    return compare_to(kOther) == -1;
  }

  template <bool ConstR>
  bool operator>(const Iterator<ConstR, IsReversed> kOther) const {
    return compare_to(kOther) == 1;
  }

  template <bool ConstR>
  bool operator<=(const Iterator<ConstR, IsReversed> kOther) const {
    return compare_to(kOther) != 1;
  }

  template <bool ConstR>
  bool operator>=(const Iterator<ConstR, IsReversed> kOther) const {
    return compare_to(kOther) != -1;
  }

  template <bool ConstR>
  bool operator==(const Iterator<ConstR, IsReversed> kOther) const {
    return compare_to(kOther) == 0;
  }

  template <bool ConstR>
  bool operator!=(const Iterator<ConstR, IsReversed> kOther) const {
    return compare_to(kOther) != 0;
  }

  template <bool ConstR>
  difference_type operator-(const Iterator<ConstR, IsReversed> kOther) const {
    return (static_cast<int>((ptr_ - kOther.ptr_) * kInnerArraySize) +
            static_cast<int>(pos_) - static_cast<int>(kOther.pos_)) *
           (1 - 2 * static_cast<int>(IsReversed));
  }

  reference operator*() const {
    if (ptr_ == nullptr) {
      throw std::runtime_error("null pointer reference");
    }

    return (*ptr_)[pos_];
  }

  pointer operator->() const {
    if (ptr_ == nullptr) {
      throw std::runtime_error("null pointer reference");
    }

    return *ptr_ + pos_;
  }
};
