add_library(third-party INTERFACE)

find_package(Vulkan REQUIRED)

add_subdirectory(SDL)
add_subdirectory(VulkanMemoryAllocator)

target_link_libraries(third-party INTERFACE Vulkan::Vulkan SDL3::SDL3 VulkanMemoryAllocator)
target_precompile_headers(third-party INTERFACE <vulkan/vulkan.hpp>)
