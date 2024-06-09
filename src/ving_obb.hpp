#pragma once

#include <array>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <ving_aabb.hpp>

namespace ving
{
struct OBB
{
    OBB(AABB aabb, glm::mat4 model_mtx);

    glm::vec3 center{};
    std::array<glm::vec3, 3> normals{}; // Side directions of the box
    glm::vec3 h{};                      // Half lengths
};
} // namespace ving
