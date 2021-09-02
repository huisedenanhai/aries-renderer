file(GLOB VULKAN_FILES
        include/vulkan/*.c
        include/vulkan/*.cpp
        include/vulkan/*.h
        include/vulkan/*.hpp
        )

add_library(vulkan
        ${VULKAN_FILES})

target_include_directories(vulkan PUBLIC include)

target_compile_definitions(vulkan PUBLIC
        VOLK_NO_DEVICE_PROTOTYPES
        VOLK_NO_INSTANCE_PROTOTYPES
        VK_NO_PROTOTYPES)