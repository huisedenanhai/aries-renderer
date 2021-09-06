#pragma once

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace ars {
template <typename T, uint32_t Capacity> struct SmallVec {
private:
  static constexpr bool MoveNoExcept = noexcept(T(std::declval<T>()));
  using SizeType = std::conditional_t<
      Capacity <= std::numeric_limits<uint8_t>::max(),
      uint8_t,
      std::conditional_t<Capacity <= std::numeric_limits<uint16_t>::max(),
                         uint16_t,
                         uint32_t>>;

public:
  SmallVec() = default;

  SmallVec(std::initializer_list<T> vl) {
    for (auto &&v : vl) {
      emplace_back(std::move(v));
    }
  }

  SmallVec(const SmallVec &vec) {
    for (const auto &v : vec) {
      push_back(v);
    }
  }

  SmallVec(SmallVec &&vec) noexcept(MoveNoExcept) {
    for (auto &&v : vec) {
      emplace_back(std::move(v));
    }
  }

  SmallVec &operator=(const SmallVec &v) {
    return assign_impl(v);
  }

  SmallVec &operator=(SmallVec &&v) noexcept(MoveNoExcept) {
    return assign_impl(std::move(v));
  }

  ~SmallVec() {
    reset();
  }

  T &operator[](size_t index) {
    assert(index < _size);
    return *get_ptr(index);
  }

  const T &operator[](size_t index) const {
    assert(index < _size);
    return *get_ptr(index);
  }

  using Iterator = T *;
  using ConstIterator = const T *;

  void push_back(const T &val) {
    assert(_size < Capacity);
    new (get_ptr(_size)) T(val);
    _size++;
  }

  void resize(size_t size) {
    assert(size <= Capacity);
    if (size < _size) {
      for (size_t i = size; i < _size; i++) {
        get_ptr(i)->~T();
      }
      _size = size;
    } else if (size > _size) {
      for (size_t i = _size; i < size; i++) {
        emplace_back(T{});
      }
    } else {
      // DO NOTHING
    }
  }

  template <typename... Args>
  void emplace_back(Args &&...args) noexcept(
      noexcept(T(std::forward<Args>(args)...))) {
    assert(_size < Capacity);
    new (get_ptr(_size)) T(std::forward<Args>(args)...);
    _size++;
  }

  Iterator begin() noexcept {
    return get_ptr(0);
  }

  ConstIterator begin() const noexcept {
    return get_ptr(0);
  }

  Iterator end() noexcept {
    return get_ptr(size());
  }

  ConstIterator end() const noexcept {
    return get_ptr(size());
  }

  [[nodiscard]] size_t size() const noexcept {
    return static_cast<size_t>(_size);
  }

  T *data() noexcept {
    return get_ptr(0);
  }

  const T *data() const noexcept {
    return get_ptr(0);
  }

private:
  template <typename Vec> SmallVec &assign_impl(Vec &&v) {
    if (this == &v) {
      return *this;
    }
    reset();
    new (this) SmallVec(std::forward<Vec>(v));
    return *this;
  }

  T *get_ptr(int32_t index) const noexcept {
    return const_cast<T *>(_data._t + index);
  }

  void reset() {
    for (T &val : *this) {
      val.~T();
    }
    _size = 0;
  }

  SizeType _size{};
  alignas(T) union Data {
    T _t[Capacity];
    char _arena[Capacity * sizeof(T)]{};
    Data() : _arena{} {}
    ~Data() = default;
  } _data;
};
} // namespace ars