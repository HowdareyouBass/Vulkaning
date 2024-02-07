file(GLOB_RECURSE project-sources "*.cpp")
file(GLOB_RECURSE project-cpp-headers "*.hpp")

add_executable(${PROJECT_NAME} ${project-sources} ${project-cpp-headers})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} third-party)

