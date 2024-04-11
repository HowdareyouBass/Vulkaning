#pragma once

#include "ving_scene.hpp"

#include <glm/vec3.hpp>
#include <utility>

namespace ving
{
struct RayHitInfo
{
    uint64_t object_id;
    glm::vec3 position;
};

std::pair<bool, RayHitInfo> raycast_scene(glm::vec3 origin, glm::vec3 direction, const Scene &scene);

} // namespace ving
