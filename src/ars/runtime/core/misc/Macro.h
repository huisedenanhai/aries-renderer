#pragma once

#define ARS_NO_COPY(cls)                                                       \
    cls(const cls &) = delete;                                                 \
    cls &operator=(const cls &) = delete

#define ARS_NO_MOVE(cls)                                                       \
    cls(cls &&) = delete;                                                      \
    cls &operator=(cls &&) = delete

#define ARS_NO_COPY_MOVE(cls)                                                  \
    ARS_NO_COPY(cls);                                                          \
    ARS_NO_MOVE(cls)

#define ARS_DEFAULT_COPY(cls)                                                  \
    cls(const cls &) = default;                                                \
    cls &operator=(const cls &) = default

#define ARS_DEFAULT_MOVE(cls)                                                  \
    cls(cls &&) = default;                                                     \
    cls &operator=(cls &&) = default

#define ARS_DEFAULT_COPY_MOVE(cls)                                             \
    ARS_DEFAULT_COPY(cls);                                                     \
    ARS_DEFAULT_MOVE(cls)

#define ARS_MACRO_CONCAT2(a, b) a##b

// Concat after a and b is expanded
#define ARS_MACRO_CONCAT(a, b) ARS_MACRO_CONCAT2(a, b)

#define ARS_MACRO_AS_STRING2(a) #a

// Add quote after 'a' is expanded
#define ARS_MACRO_AS_STRING(a) ARS_MACRO_AS_STRING2(a)

// For padding struct fields
#define ARS_PADDING_FIELD(ty)                                                  \
    [[maybe_unused]] ty ARS_MACRO_CONCAT(_unused_field_, __LINE__)
