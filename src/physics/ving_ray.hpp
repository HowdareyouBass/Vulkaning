#pragma once

#include <glm/vec3.hpp>

namespace ving
{
struct RayHitInfo
{
    glm::vec3 position;
};

bool raycast_scene(glm::vec3 direction, RayHitInfo &hit_info);
} // namespace ving
