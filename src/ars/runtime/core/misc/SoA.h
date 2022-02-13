#pragma once

#include <cassert>
#include <optional>
#include <tuple>
#include <vector>

namespace ars {
class IndexPool {
  public:
    uint64_t alloc();

    void free(uint64_t index);

    void set_value(uint64_t index, uint64_t value);

    [[nodiscard]] uint64_t get_value(uint64_t index) const;

    void clear();

  private:
    struct IndexSlot {
      public:
        static constexpr uint64_t FREE_BIT = 1ULL << 63;

        [[nodiscard]] std::optional<uint64_t> get_alloc() const;

        [[nodiscard]] std::optional<uint64_t> get_free() const;

        void set_alloc(uint64_t v);

        void set_free(uint64_t v);

        [[nodiscard]] bool is_free() const;

      private:
        uint64_t _value{};
    };

    std::vector<IndexSlot> _indices{};
    std::optional<uint64_t> _free_index{};
};

template <typename T, typename... Ts> struct IndexOfType;

template <typename T, typename Hd, typename... Tl>
struct IndexOfType<T, Hd, Tl...> {
    static constexpr size_t value =
        std::is_same_v<T, Hd> ? 0 : 1 + IndexOfType<T, Tl...>::value;
};

template <typename T> struct IndexOfType<T> {
    static constexpr size_t value = 0;
};

template <typename Func, typename Tp>
void tuple_for_each(Func &&func, Tp &&tp) {
    std::apply([&](auto &&...v) { (func(std::forward<decltype(v)>(v)), ...); },
               std::forward<Tp>(tp));
}

template <typename... Ts> struct SoA {
  public:
    struct Id {
      public:
        Id() = default;

        bool operator==(const Id &rhs) const {
            return _value == rhs._value;
        }

        bool operator!=(const Id &rhs) const {
            return !(*this == rhs);
        }

        bool operator<(const Id &rhs) const {
            return _value < rhs._value;
        }

        [[nodiscard]] uint64_t value() const {
            return _value;
        }

        bool valid() const {
            return *this != Id{};
        }

      private:
        friend SoA;

        explicit Id(uint64_t value) : _value(value) {}

        uint64_t _value = ~0ULL;
    };

    Id alloc() {
        auto id = _indices.alloc();
        _indices.set_value(id, static_cast<uint64_t>(size()));
        tuple_for_each([&](auto &&v) { v.emplace_back(); }, _soa);
        get_inverse_id().back().value = id;
        return Id{id};
    }

    void free(Id id) {
        if (!id.valid()) {
            return;
        }
        auto soa_index = _indices.get_value(id._value);
        auto moved_id = get_inverse_id().back().value;
        tuple_for_each(
            [&](auto &&v) {
                std::swap(v[soa_index], v.back());
                v.pop_back();
            },
            _soa);
        _indices.set_value(moved_id, soa_index);
        _indices.free(id._value);
    }

    [[nodiscard]] size_t size() const {
        return static_cast<uint64_t>(get_inverse_id().size());
    }

    template <typename T> const T *get_array() const {
        return get_array_impl<T>();
    }

    template <typename T> T *get_array() {
        return const_cast<T *>(get_array_impl<T>());
    }

    template <typename T> T &get(Id id) {
        assert(id.valid());
        auto soa_index = _indices.get_value(id._value);
        return get_array<T>()[soa_index];
    }

    template <typename T> const T &get(Id id) const {
        assert(id.valid());
        auto soa_index = _indices.get_value(id._value);
        return get_array<T>()[soa_index];
    }

    template <typename Func> void for_each_id(Func &&func) const {
        auto count = size();
        for (size_t i = 0; i < count; i++) {
            auto id = Id(get_inverse_id()[i].value);
            func(id);
        }
    }

    void clear() {
        tuple_for_each([&](auto &&v) { v.clear(); }, _soa);
        _indices.clear();
    }

  private:
    struct InverseId {
        uint64_t value;
    };

    std::vector<InverseId> &get_inverse_id() {
        return std::get<0>(_soa);
    }

    const std::vector<InverseId> &get_inverse_id() const {
        return std::get<0>(_soa);
    }

    template <typename T> const T *get_array_impl() const {
        constexpr auto tid = IndexOfType<T, Ts...>::value;
        static_assert(sizeof...(Ts) != tid,
                      "Type not found in the SoA type list");

        // the first slot is for InverseId
        return std::get<tid + 1>(_soa).data();
    }

    std::tuple<std::vector<InverseId>, std::vector<Ts>...> _soa{};
    IndexPool _indices{};
};
} // namespace ars