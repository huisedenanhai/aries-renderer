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
