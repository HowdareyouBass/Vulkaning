#pragma once

#include "ving_camera.hpp"
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

enum GizmoType
{
    X,
    Y,
    Z
};

// NOTE: Might be bad to use pairs
std::pair<bool, RayHitInfo> raycast_scene(glm::vec3 origin, glm::vec3 direction, const Scene &scene);
std::pair<bool, GizmoType> raycast_gizmos(glm::vec3 origin, glm::vec3 direction, const SceneObject &object);
std::pair<bool, GizmoType> raycast_gizmos(glm::vec2 mouse_position, const PerspectiveCamera &camera,
                                          const SceneObject &object);

} // namespace ving
