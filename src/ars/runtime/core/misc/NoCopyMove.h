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
