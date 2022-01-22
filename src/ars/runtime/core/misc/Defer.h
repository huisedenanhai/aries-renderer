#pragma once

#include "Macro.h"
#include <utility>

namespace ars {
namespace detail {
template <typename Func> struct DeferHandle {
    explicit DeferHandle(Func &&f) : func(std::move(f)) {}
    ARS_NO_COPY_MOVE(DeferHandle);
    ~DeferHandle() {
        func();
    }

    Func func;
};
} // namespace detail

template <typename Func> auto make_defer(Func &&func) {
    return detail::DeferHandle<std::remove_reference_t<std::remove_cv_t<Func>>>(
        std::forward<Func>(func));
}

#define ARS_DEFER_TAGGED(tag, func)                                            \
    auto ARS_NAME_WITH_LINENO(defer_handle_##tag) = ars::make_defer(func)

#define ARS_DEFER(func) ARS_DEFER_TAGGED(, func)

} // namespace ars