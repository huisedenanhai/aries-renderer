#pragma once

#include <utility>

namespace ars {
template <typename... Fs> struct Visitor : Fs... {
    template <typename... Ts>
    explicit Visitor(Ts &&...ts) : Fs{std::template forward<Ts>(ts)}... {}

    using Fs::operator()...;
};

template <typename... Fs> auto make_visitor(Fs &&...fs) {
    return Visitor<std::decay_t<Fs>...>(std::forward<Fs>(fs)...);
}
} // namespace ars