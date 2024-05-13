#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace ving
{
struct AABB
{
    constexpr static float minimum_value = 0.03f;

    static AABB to_world_space(const glm::mat4 &pvm_matrix, AABB aabb);

    glm::vec3 min{std::numeric_limits<float>::infinity()}, max{-std::numeric_limits<float>::infinity()};
};
} // namespace ving
