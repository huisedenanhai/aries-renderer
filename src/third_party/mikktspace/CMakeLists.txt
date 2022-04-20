add_library(mikktspace
        include/mikktspace/mikktspace.c
        include/mikktspace/mikktspace.h)

target_include_directories(mikktspace PUBLIC
        include)