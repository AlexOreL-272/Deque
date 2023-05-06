#include <exception>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T, typename Alloc = std::allocator<T>>
class Deque {
 private:
  std::vector<T*> outer_;

  size_t num_of_arrays_above_ = 1;
  size_t num_of_arrays_beyond_ = 2;
  size_t actual_size_ = 0;

  T** begin_vector_ = nullptr;
  size_t begin_pos_ = 0;

  T** end_vector_ = nullptr;
  size_t end_pos_ = 0;

  static const size_t kInnerArraySize = 1000;

  Alloc alloc_;
  using alloc_traits = std::allocator_traits<Alloc>;

 public:
  template <bool IsConst, bool IsReversed>
  class Iterator;

  using const_iterator = Iterator<true, false>;
  using iterator = Iterator<false, false>;
  using reverse_iterator = Iterator<false, true>;
  using const_reverse_iterator = Iterator<true, true>;

  Deque() {
    outer_.resize(3, nullptr);
    outer_[1] = alloc_traits::allocate(alloc_, kInnerArraySize);
    begin_vector_ = &outer_[1];
    end_vector_ = &outer_[1];
  }

  explicit Deque(const Alloc& alloc) {
    outer_.resize(3, nullptr);
    outer_[1] = alloc_traits::allocate(alloc, kInnerArraySize);
    begin_vector_ = &outer_[1];
    end_vector_ = &outer_[1];
  }

  void full_destroy(T** from_vec, size_t from_pos, T** to_vec, size_t to_pos) {
    if (from_vec == nullptr || to_vec == nullptr) {
      return;
    }

    for (; from_vec < to_vec; ++from_vec) {
      for (; from_pos < kInnerArraySize; ++from_pos) {
        alloc_traits::destroy(alloc_, *from_vec + from_pos);
      }
      alloc_traits::deallocate(alloc_, *from_vec, kInnerArraySize);
      from_pos = 0;
    }

    for (; from_pos < to_pos; ++from_pos) {
      alloc_traits::destroy(alloc_, *to_vec + from_pos);
    }
    alloc_traits::deallocate(alloc_, *to_vec, kInnerArraySize);

    actual_size_ = 0;
    begin_vector_ = nullptr;
    begin_pos_ = 0;
    end_vector_ = nullptr;
    end_pos_ = 0;
  }

  explicit Deque(const size_t& count, const Alloc& alloc = Alloc())
      : actual_size_(count), alloc_(alloc) {
    size_t num_of_arrays;
    size_t delete_to_vec;
    size_t delete_to_pos = 0;

    try {
      auto uses_whole_number =
          static_cast<size_t>(count % kInnerArraySize == 0);

      num_of_arrays = count / kInnerArraySize + 1 - uses_whole_number;
      delete_to_vec = num_of_arrays - 1;
      outer_.resize(num_of_arrays * 3, nullptr);

      num_of_arrays_above_ = num_of_arrays;
      num_of_arrays_beyond_ = num_of_arrays;

      for (size_t i = num_of_arrays;
           i < 2 * num_of_arrays - 1 + uses_whole_number;
           ++i, ++delete_to_vec) {
        outer_[i] = alloc_traits::allocate(alloc_, kInnerArraySize);

        for (size_t j = 0; j < kInnerArraySize; ++j, ++delete_to_pos) {
          alloc_traits::construct(alloc_, outer_[i] + j);
        }
        delete_to_pos = 0;
      }

      outer_[2 * num_of_arrays - 1 + uses_whole_number] =
          alloc_traits::allocate(alloc_, kInnerArraySize);

      end_vector_ = &outer_[2 * num_of_arrays - 1 + uses_whole_number];
      end_pos_ = count % kInnerArraySize;

      begin_vector_ = &outer_[num_of_arrays];
      begin_pos_ = 0;

      ++delete_to_vec;
      for (size_t i = 0; i < end_pos_; ++i, ++delete_to_pos) {
        alloc_traits::construct(alloc_, *end_vector_ + i);
      }
    } catch (...) {
      full_destroy(&outer_[num_of_arrays], 0, &outer_[delete_to_vec],
                   delete_to_pos);
      throw -1;  // std::runtime_error("Could not constuct an object\n");
    }
  }

  Deque(const size_t& count, const T& value, const Alloc& alloc = Alloc())
      : actual_size_(count), alloc_(alloc) {
    size_t num_of_arrays;
    size_t delete_to_vec;
    size_t delete_to_pos = 0;

    try {
      auto uses_whole_number =
          static_cast<size_t>(count % kInnerArraySize == 0);

      num_of_arrays = count / kInnerArraySize + 1 - uses_whole_number;
      delete_to_vec = num_of_arrays - 1;
      outer_.resize(num_of_arrays * 3, nullptr);

      num_of_arrays_above_ = num_of_arrays;
      num_of_arrays_beyond_ = num_of_arrays;

      for (size_t i = num_of_arrays;
           i < 2 * num_of_arrays - 1 + uses_whole_number;
           ++i, ++delete_to_vec) {
        outer_[i] = alloc_traits::allocate(alloc_, kInnerArraySize);

        for (size_t j = 0; j < kInnerArraySize; ++j, ++delete_to_pos) {
          alloc_traits::construct(alloc_, outer_[i] + j, value);
        }
        delete_to_pos = 0;
      }

      outer_[2 * num_of_arrays - 1 + uses_whole_number] =
          alloc_traits::allocate(alloc_, kInnerArraySize);
      end_vector_ = &outer_[2 * num_of_arrays - 1 + uses_whole_number];
      end_pos_ = count % kInnerArraySize;

      begin_vector_ = &outer_[num_of_arrays];
      begin_pos_ = 0;

      ++delete_to_vec;
      for (size_t i = 0; i < end_pos_; ++i, ++delete_to_pos) {
        alloc_traits::construct(alloc_, *end_vector_ + i, value);
      }
    } catch (...) {
      full_destroy(&outer_[num_of_arrays], 0, &outer_[delete_to_vec],
                   delete_to_pos);
      throw -1;  // std::runtime_error("Could not constuct an object\n");
    }
  }

  Deque(const Deque& other) {
    size_t size;
    size_t delete_to_vec;
    size_t delete_to_pos = 0;
    alloc_ = alloc_traits::select_on_container_copy_construction(other.alloc_);

    if (other.empty()) {
      outer_.resize(3, nullptr);
      outer_[1] = alloc_traits::allocate(alloc_, kInnerArraySize);
      begin_vector_ = &outer_[1];
      end_vector_ = &outer_[1];
      return;
    }

    try {
      size = other.outer_.size();
      auto uses_whole_number =
          static_cast<size_t>(other.size() % kInnerArraySize == 0);
      outer_.resize(size, nullptr);

      num_of_arrays_above_ = other.num_of_arrays_above_;
      num_of_arrays_beyond_ = other.num_of_arrays_beyond_;
      actual_size_ = other.actual_size_;
      delete_to_vec = num_of_arrays_above_ - 1;

      for (size_t i = num_of_arrays_above_;
           i < size - num_of_arrays_beyond_ - 1 + uses_whole_number;
           ++i, ++delete_to_vec) {
        outer_[i] = alloc_traits::allocate(alloc_, kInnerArraySize);
        for (size_t j = i == num_of_arrays_above_ ? other.begin_pos_ : 0;
             j < kInnerArraySize; ++j, ++delete_to_pos) {
          alloc_traits::construct(alloc_, outer_[i] + j, other.outer_[i][j]);
        }
        delete_to_pos = 0;
      }
      begin_vector_ = &outer_[num_of_arrays_above_];
      begin_pos_ = other.begin_pos_;
      if (num_of_arrays_beyond_ + 1 - uses_whole_number != 0) {
        outer_[size - num_of_arrays_beyond_ - 1 + uses_whole_number] =
            alloc_traits::allocate(alloc_, kInnerArraySize);
        end_vector_ =
            &outer_[size - num_of_arrays_beyond_ - 1 + uses_whole_number];
      } else {
        end_vector_ = &outer_[size - 1] + 1;
      }

      end_pos_ = other.end_pos_;
      if (*other.end_vector_ != nullptr) {
        ++delete_to_vec;
        for (size_t i = begin_vector_ == end_vector_ ? begin_pos_ : 0;
             i < end_pos_; ++i, ++delete_to_pos) {
          alloc_traits::construct(alloc_, *end_vector_ + i,
                                  (*other.end_vector_)[i]);
        }
      }
    } catch (...) {
      full_destroy(&outer_[num_of_arrays_above_], 0, &outer_[delete_to_vec],
                   delete_to_pos);
      throw -1;  // std::runtime_error("Could not copy-constuct an object\n");
    }
  }

  Deque(Deque&& other) noexcept {
    alloc_ = std::move(other.alloc_);

    std::swap(outer_, other.outer_);

    std::swap(num_of_arrays_above_, other.num_of_arrays_above_);
    std::swap(num_of_arrays_beyond_, other.num_of_arrays_beyond_);
    std::swap(actual_size_, other.actual_size_);

    std::swap(begin_vector_, other.begin_vector_);
    std::swap(begin_pos_, other.begin_pos_);
    std::swap(end_vector_, other.end_vector_);
    std::swap(end_pos_, other.end_pos_);
  }

  Deque(std::initializer_list<T> init, const Alloc& alloc = Alloc())
      : actual_size_(init.size()), alloc_(alloc) {
    size_t num_of_arrays;
    size_t delete_to_vec;
    size_t delete_to_pos = 0;
    size_t size = init.size();

    try {
      auto init_beg = init.begin();
      auto uses_whole_number = static_cast<size_t>(size % kInnerArraySize == 0);

      num_of_arrays = size / kInnerArraySize + uses_whole_number + 1;
      delete_to_vec = num_of_arrays;
      outer_.resize(num_of_arrays * 3, nullptr);

      num_of_arrays_above_ = num_of_arrays;
      num_of_arrays_beyond_ = num_of_arrays;

      for (size_t i = num_of_arrays;
           i < 2 * num_of_arrays - 1 - uses_whole_number;
           ++i, ++delete_to_vec) {
        outer_[i] = alloc_traits::allocate(alloc_, kInnerArraySize);

        for (size_t j = 0; j < kInnerArraySize; ++j, ++delete_to_pos) {
          alloc_traits::construct(alloc_, outer_[i] + j, *init_beg++);
        }
        delete_to_pos = 0;
      }

      outer_[2 * num_of_arrays - 1 - uses_whole_number] =
          alloc_traits::allocate(alloc_, kInnerArraySize);

      end_vector_ = &outer_[2 * num_of_arrays - 1 - uses_whole_number];
      end_pos_ = size % kInnerArraySize;

      begin_vector_ = &outer_[num_of_arrays];
      begin_pos_ = 0;

      ++delete_to_vec;
      for (size_t i = 0; i < end_pos_; ++i, ++delete_to_pos) {
        alloc_traits::construct(alloc_, *end_vector_ + i, *init_beg++);
      }
    } catch (...) {
      full_destroy(&outer_[num_of_arrays], 0, &outer_[delete_to_vec],
                   delete_to_pos);
      throw -1;  // std::runtime_error("Could not constuct an object\n");
    }
  }

  ~Deque() { full_destroy(begin_vector_, begin_pos_, end_vector_, end_pos_); }

  Deque& operator=(const Deque& obj_to_assign) {
    Deque copy(obj_to_assign);  // but it might throw an exception there...
    if (alloc_traits::propagate_on_container_copy_assignment::value &&
        alloc_ != obj_to_assign.alloc_) {
      alloc_ = obj_to_assign.alloc_;
    }

    std::swap(outer_, copy.outer_);

    std::swap(num_of_arrays_above_, copy.num_of_arrays_above_);
    std::swap(num_of_arrays_beyond_, copy.num_of_arrays_beyond_);
    std::swap(actual_size_, copy.actual_size_);

    std::swap(begin_vector_, copy.begin_vector_);
    std::swap(begin_pos_, copy.begin_pos_);
    std::swap(end_vector_, copy.end_vector_);
    std::swap(end_pos_, copy.end_pos_);

    return *this;
  }

  Deque& operator=(Deque&& other) noexcept {
    if (*this != other) {
      alloc_ = std::move(other.alloc_);

      std::swap(outer_, other.outer_);

      std::swap(num_of_arrays_above_, other.num_of_arrays_above_);
      std::swap(num_of_arrays_beyond_, other.num_of_arrays_beyond_);
      std::swap(actual_size_, other.actual_size_);

      std::swap(begin_vector_, other.begin_vector_);
      std::swap(begin_pos_, other.begin_pos_);
      std::swap(end_vector_, other.end_vector_);
      std::swap(end_pos_, other.end_pos_);

      other.full_destroy(other.begin_vector_, other.begin_pos_,
                         other.end_vector_, other.end_pos_);
    }

    return *this;
  }

  size_t size() const noexcept { return actual_size_; }

  bool empty() const noexcept { return actual_size_ == 0; }

  T& operator[](size_t pos) {
    if (begin_pos_ + pos < kInnerArraySize) {
      return (*begin_vector_)[begin_pos_ + pos];
    }

    pos -= kInnerArraySize - begin_pos_;
    size_t num_of_arrays_to_skip = pos / kInnerArraySize + 1;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  const T& operator[](size_t pos) const {
    if (begin_pos_ + pos < kInnerArraySize) {
      return (*begin_vector_)[begin_pos_ + pos];
    }

    pos -= kInnerArraySize - begin_pos_;
    size_t num_of_arrays_to_skip = pos / kInnerArraySize + 1;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  T& at(size_t pos) {
    if (pos >= actual_size_) {
      throw std::out_of_range("Invalid index!\n");
    }

    if (begin_pos_ + pos < kInnerArraySize) {
      return (*begin_vector_)[begin_pos_ + pos];
    }

    pos -= kInnerArraySize - begin_pos_;
    size_t num_of_arrays_to_skip = pos / kInnerArraySize + 1;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  const T& at(size_t pos) const {
    if (pos >= actual_size_) {
      throw std::out_of_range("Invalid index!\n");
    }

    if (begin_pos_ + pos < kInnerArraySize) {
      return (*begin_vector_)[begin_pos_ + pos];
    }

    pos -= kInnerArraySize - begin_pos_;
    size_t num_of_arrays_to_skip = pos / kInnerArraySize + 1;
    size_t inner_array_pos = pos % kInnerArraySize;

    return (*(begin_vector_ + num_of_arrays_to_skip))[inner_array_pos];
  }

  Alloc get_allocator() { return alloc_; }

  void resize_if_needed() {
    size_t cur_size = outer_.size();

    if (!(end_vector_ > &outer_[cur_size - 1] && end_pos_ == 0) &&
        !(begin_vector_ < (outer_).data() && begin_pos_ == 0)) {
      return;
    }

    std::vector<T*> new_vec(cur_size * 3, nullptr);
    for (size_t i = cur_size - 1; i < 2 * cur_size - 1; ++i) {
      new_vec[i] = outer_[i - cur_size + 1];
    }
    new_vec[2 * cur_size - 1] = alloc_traits::allocate(alloc_, kInnerArraySize);

    num_of_arrays_above_ += cur_size - 1;
    num_of_arrays_beyond_ += cur_size + 1;

    auto addr = (outer_).data();
    outer_ = new_vec;

    begin_vector_ = &outer_[cur_size - 1] + (begin_vector_ - addr);
    end_vector_ = &outer_[cur_size - 1] + (end_vector_ - addr);
  }

  void push_back(const T& elem) {
    resize_if_needed();

    if (nullptr == *end_vector_) {
      *end_vector_ = alloc_traits::allocate(alloc_, kInnerArraySize);
    }

    try {
      alloc_traits::construct(alloc_, *end_vector_ + end_pos_, elem);
    } catch (...) {
      throw -2;  // std::runtime_error("Could not push back this value\n");
    }

    end_pos_++;
    actual_size_++;

    if (end_pos_ == kInnerArraySize) {
      end_pos_ = 0;
      end_vector_++;
    }
    if (end_pos_ == 1) {
      if (num_of_arrays_beyond_ == 0) {
        std::cout << "<<<<<<<<<<<< Some kind of error: >>>>>>>>>>>>\n";
      } else {
        num_of_arrays_beyond_--;
      }
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
        *begin_vector_ = alloc_traits::allocate(alloc_, kInnerArraySize);
      }
    }

    try {
      begin_pos_--;
      alloc_traits::construct(alloc_, *begin_vector_ + begin_pos_, elem);
    } catch (...) {
      begin_pos_ = copy_pos;
      begin_vector_ = copy_begin;
      throw -2;  // std::runtime_error("Could not push_front an object\n");
    }

    actual_size_++;
  }

  void pop_back() {
    if (actual_size_ == 0) {
      return;
    }

    if (end_pos_ == 0) {
      alloc_traits::destroy(alloc_, *end_vector_);
      end_pos_ = kInnerArraySize - 1;
      end_vector_--;
      num_of_arrays_beyond_++;
      alloc_traits::deallocate(alloc_, *(end_vector_ + 1), kInnerArraySize);
      *(end_vector_ + 1) = nullptr;
    } else {
      alloc_traits::destroy(alloc_, *end_vector_ + end_pos_ - 1);
      end_pos_--;
    }

    actual_size_--;
  }

  void pop_front() {
    if (actual_size_ == 0) {
      return;
    }

    alloc_traits::destroy(alloc_, *begin_vector_ + begin_pos_);

    begin_pos_++;
    if (begin_pos_ == kInnerArraySize) {
      begin_pos_ = 0;
      begin_vector_++;
      num_of_arrays_above_++;
      alloc_traits::deallocate(alloc_, *(begin_vector_ - 1), kInnerArraySize);
      *(begin_vector_ - 1) = nullptr;
    }

    actual_size_--;
  }

  void push_back(T&& elem) {
    resize_if_needed();

    if (nullptr == *end_vector_) {
      *end_vector_ = alloc_traits::allocate(alloc_, kInnerArraySize);
    }

    try {
      alloc_traits::construct(alloc_, *end_vector_ + end_pos_, std::move(elem));
    } catch (...) {
      throw -2;  // std::runtime_error("Could not push back this value\n");
    }

    end_pos_++;
    actual_size_++;

    if (end_pos_ == kInnerArraySize) {
      end_pos_ = 0;
      end_vector_++;
    }
    if (end_pos_ == 1) {
      num_of_arrays_beyond_--;
    }

    resize_if_needed();
  }

  void push_front(T&& elem) {
    resize_if_needed();

    size_t copy_pos = begin_pos_;
    T** copy_begin = begin_vector_;

    if (begin_pos_ == 0) {
      begin_pos_ = kInnerArraySize;
      begin_vector_--;
      num_of_arrays_above_--;

      if (*begin_vector_ == nullptr) {
        *begin_vector_ = alloc_traits::allocate(alloc_, kInnerArraySize);
      }
    }

    try {
      begin_pos_--;
      alloc_traits::construct(alloc_, *begin_vector_ + begin_pos_,
                              std::move(elem));
    } catch (...) {
      begin_pos_ = copy_pos;
      begin_vector_ = copy_begin;
      throw -2;  // std::runtime_error("Could not push_front an object\n");
    }

    actual_size_++;
  }

  template <class... Args>
  void emplace_back(Args&&... elems) {
    resize_if_needed();

    if (*end_vector_ == nullptr) {
      *end_vector_ = alloc_traits::allocate(alloc_, kInnerArraySize);
    }

    try {
      alloc_traits::construct(alloc_, *end_vector_ + end_pos_,
                              std::forward<Args>(elems)...);
    } catch (...) {
      throw -2;  // std::runtime_error("Could not push back this value\n");
    }

    end_pos_++;
    actual_size_++;

    if (end_pos_ == kInnerArraySize) {
      end_pos_ = 0;
      end_vector_++;
      num_of_arrays_beyond_--;
    }

    resize_if_needed();
  }

  template <class... Args>
  void emplace_front(Args&&... elems) {
    resize_if_needed();

    size_t copy_pos = begin_pos_;
    T** copy_begin = begin_vector_;

    if (begin_pos_ == 0) {
      begin_pos_ = kInnerArraySize;
      begin_vector_--;
      num_of_arrays_above_--;

      if (*begin_vector_ == nullptr) {
        *begin_vector_ = alloc_traits::allocate(alloc_, kInnerArraySize);
      }
    }

    try {
      begin_pos_--;
      alloc_traits::construct(alloc_, *begin_vector_ + begin_pos_,
                              std::forward<Args>(elems)...);
    } catch (...) {
      begin_pos_ = copy_pos;
      begin_vector_ = copy_begin;
      throw -2;  // std::runtime_error("Could not push_front an object\n");
    }

    actual_size_++;
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
      std::cout << "---------- Error! -------------\n";
      *this = copy;
      throw -3;  // std::runtime_error("Could not insert a value\n");
    }
  }

  void erase(iterator iter) {
    Deque copy(*this);

    try {
      auto end_iter = end();

      while (iter < end_iter) {
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

  void emplace(iterator iter, T&& elem) {
    Deque copy(*this);

    try {
      auto end_iter = end();

      T tmp1(std::move(elem));
      T tmp2{};

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
};

template <typename T, typename Alloc>
template <bool IsConst, bool IsReversed>
class Deque<T, Alloc>::Iterator {
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