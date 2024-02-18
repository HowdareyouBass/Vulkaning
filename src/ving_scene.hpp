#pragma once

#include "glm/geometric.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ving
{
struct Scene
{
    // NOTE: W for light intencity
    glm::vec4 light_direction{glm::normalize(glm::vec3{0.4f, 0.6f, -0.23f}), 1.5f};
};
} // namespace ving
