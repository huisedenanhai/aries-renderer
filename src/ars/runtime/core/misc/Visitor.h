#pragma once

#include <utility>
#include <variant>

namespace ars {
template <typename... Fs> struct Visitor : Fs... {
    template <typename... Ts>
    explicit Visitor(Ts &&...ts) : Fs{std::template forward<Ts>(ts)}... {}

    using Fs::operator()...;
};

template <typename... Fs> auto make_visitor(Fs &&...fs) {
    return Visitor<std::decay_t<Fs>...>(std::forward<Fs>(fs)...);
}

namespace detail {
template <typename... Ts>
constexpr std::variant<Ts...> &as_variant(std::variant<Ts...> &v) noexcept {
    return v;
}

template <typename... Ts>
constexpr const std::variant<Ts...> &
as_variant(const std::variant<Ts...> &v) noexcept {
    return v;
}

template <typename... Ts>
constexpr std::variant<Ts...> &&as_variant(std::variant<Ts...> &&v) noexcept {
    return std::move(v);
}

template <typename... Ts>
constexpr const std::variant<Ts...> &&
as_variant(const std::variant<Ts...> &&v) noexcept {
    return std::move(v);
}
} // namespace detail

template <typename VisitorT, typename... Vs>
decltype(auto) visit(VisitorT &&visitor, Vs &&...var) {
    // Work around for gcc headers, where std::visitor does not accept inherit
    // types of std::variant
    return std::visit(std::forward<VisitorT>(visitor),
                      detail::as_variant(std::forward<Vs>(var))...);
}
} // namespace ars