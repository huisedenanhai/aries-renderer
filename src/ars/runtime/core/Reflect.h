#pragma once

#include <rttr/rttr_enable.h>

namespace ars {}

// Almost the same as RTTR_ENABLE, fix warning on some compiler with 'override'
#define RTTR_DERIVE(...)                                                       \
  public:                                                                      \
    rttr::type get_type() const override {                                     \
        return rttr::detail::get_type_from_instance(this);                     \
    }                                                                          \
    void *get_ptr() override {                                                 \
        return reinterpret_cast<void *>(this);                                 \
    }                                                                          \
    rttr::detail::derived_info get_derived_info() override {                   \
        return {reinterpret_cast<void *>(this),                                \
                rttr::detail::get_type_from_instance(this)};                   \
    }                                                                          \
    using base_class_list = TYPE_LIST(__VA_ARGS__);                            \
                                                                               \
  private:
