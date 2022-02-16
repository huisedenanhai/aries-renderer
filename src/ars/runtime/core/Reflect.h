#pragma once

#include <rttr/rttr_enable.h>

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

// Utility macros to make you type less
#define RTTR_MEMBER_PROPERTY(ty, prop)                                         \
    property(#prop, &ty::prop, &ty::set_##prop)

#define RTTR_MEMBER_PROPERTY_READONLY(ty, prop)                                \
    property_readonly(#prop, &ty::prop)

#define RTTR_ENUM_VALUE(ty, v) rttr::value(#v, ty::v)
