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

    uint64_t get_value(uint64_t index);

  private:
    struct IndexSlot {
      public:
        static constexpr uint64_t FREE_BIT = 1ULL << 63;

        std::optional<uint64_t> get_alloc();

        std::optional<uint64_t> get_free() const;

        void set_alloc(uint64_t v);

        void set_free(uint64_t v);

        bool is_free() const;

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

template <typename... Ts> struct SoA {
  public:
    struct Id {
      private:
        uint64_t _value;
    };

    Id alloc() {
        auto id = _indices.alloc();
        _indices.set_value(id, static_cast<uint64_t>(size()));
        std::apply([&](auto &&v...) { v.emplace_back(); }, _soa);
        get_inverse_id().back() = id;
        return {id};
    }

    void free(Id id) {
        auto soa_index = _indices.get_value(id._value);
        auto moved_id = get_inverse_id().back();
        std::apply(
            [&](auto &&v...) {
                std::swap(v[soa_index], v.back());
                v.pop_back();
            },
            _soa);
        _indices.set_value(moved_id, soa_index);
        _indices.free(id._value);
    }

    size_t size() const {
        return static_cast<uint64_t>(get_inverse_id().size());
    }

    template <typename T> const T *get_array() const {
        return get_array_impl<T>();
    }

    template <typename T> T *get_array() {
        return const_cast<T *>(get_array_impl<T>());
    }

    template <typename T> T &get(Id id) {
        auto soa_index = _indices.get_value(id._value);
        return get_array<T>()[soa_index];
    }

    template <typename T> const T &get(Id id) const {
        auto soa_index = _indices.get_value(id._value);
        return get_array<T>()[soa_index];
    }

  private:
    template <typename T> const T *get_array_impl() const {
        constexpr auto tid = IndexOfType<T, Ts...>::value;
        static_assert(sizeof...(Ts) != tid,
                      "Type not found in the SoA type list");
        return std::get<tid>(_soa).data();
    }

    struct InverseId {
        uint64_t value;
    };

    std::vector<InverseId> &get_inverse_id() {
        return std::get<0>(_soa);
    }

    const std::vector<InverseId> &get_inverse_id() const {
        return std::get<0>(_soa);
    }

    std::tuple<std::vector<InverseId>, std::vector<Ts>...> _soa{};
    IndexPool _indices{};
};
} // namespace ars