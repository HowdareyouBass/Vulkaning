#pragma once

#include "glm/geometric.hpp"
#include <glm/vec4.hpp>

namespace ving
{
struct Scene
{
    // NOTE: W for light intencity
    glm::vec4 light_direction{glm::normalize(glm::vec4{0.5f, 0.5f, 0.0f, 0.3f})};
};
} // namespace ving
