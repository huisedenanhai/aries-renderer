#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace ars {
template <typename T> struct Span {
    using iterator = T *;

    const T *data() const {
        return _data;
    }

    T *data() {
        return _data;
    }

    size_t size() const {
        return _size;
    }

    Span() = default;
    Span(T *data, size_t size) : _data(data), _size(size) {}

    template <typename Container>
    Span(Container &&v) : _data(std::data(v)), _size(std::size(v)) {}

    iterator begin() const {
        return _data;
    }

    iterator end() const {
        return _data + _size;
    }

    T &operator[](size_t i) {
        return _data[i];
    }

    const T &operator[](size_t i) const {
        return _data[i];
    }

    bool empty() const {
        return _data == nullptr || _size == 0;
    }

  private:
    T *_data = nullptr;
    size_t _size = 0;
};
} // namespace ars