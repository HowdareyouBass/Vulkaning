add_library(third-party INTERFACE)

find_package(Vulkan REQUIRED)

add_subdirectory(SDL)
add_subdirectory(VulkanMemoryAllocator)
add_subdirectory(glm)
add_subdirectory(KTX-Software)
add_subdirectory(json)

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)

add_subdirectory(tinygltf)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE
    imgui/imgui.h
    imgui/imgui.cpp

    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_vulkan.cpp
    imgui/backends/imgui_impl_sdl3.cpp
)

add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE tinyobjloader)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL3::SDL3)

target_link_libraries(third-party INTERFACE Vulkan::Vulkan SDL3::SDL3 imgui glm tinyobjloader ktx nlohmann_json tinygltf)
