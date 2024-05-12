#pragma once

#include "ving_camera.hpp"
#include "ving_gizmos.hpp"
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

// NOTE: Might be bad to use pairs
std::pair<bool, RayHitInfo> raycast_scene(glm::vec3 origin, glm::vec3 direction, const Scene &scene);
std::pair<bool, RayHitInfo> raycast_scene(float mouse_position_x, float mouse_position_y,
                                          const PerspectiveCamera &camera, const Scene &scene);
std::pair<bool, editor::Gizmo::Type> raycast_gizmos(glm::vec3 origin, glm::vec3 direction, const SceneObject &object);
std::pair<bool, editor::Gizmo::Type> raycast_gizmos(float mouse_position_x, float mouse_position_y,
                                                    const PerspectiveCamera &camera, const SceneObject &object);

} // namespace ving
