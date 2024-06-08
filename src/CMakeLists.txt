file(GLOB_RECURSE project-sources "*.cpp")
file(GLOB_RECURSE project-cpp-headers "*.hpp")

add_executable(${PROJECT_NAME} ${project-sources} ${project-cpp-headers})

target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_RADIANS)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} third-party)

target_precompile_headers(${PROJECT_NAME} INTERFACE <vulkan/vulkan.hpp>)

target_include_directories(${PROJECT_NAME} PRIVATE "renderers")
target_include_directories(${PROJECT_NAME} PRIVATE ".")
target_include_directories(${PROJECT_NAME} PRIVATE "editor")
target_include_directories(${PROJECT_NAME} PRIVATE "debug")
