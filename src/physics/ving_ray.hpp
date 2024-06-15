#pragma once

#include "ving_camera.hpp"
#include "ving_gizmos.hpp"
#include "ving_scene.hpp"

#include <glm/vec3.hpp>

namespace ving
{
struct RaycastInfo
{
    bool hit{false};
    float distance_from_origin;
    glm::vec3 position{};
};
struct SceneRaycastInfo
{
    bool hit;
    uint64_t object_id;
    glm::vec3 position;
};
struct GizmoRaycastInfo
{
    bool hit;
    editor::Gizmo::Coordinate coordinate;
    float offset;
};

// NOTE: Might be bad to use pairs
RaycastInfo raycast_plane(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 plane_normal,
                          float plane_normal_scalar);
RaycastInfo raycast_plane(float mouse_position_x, float mouse_position_y, const PerspectiveCamera &camera,
                          glm::vec3 plane_normal, float plane_normal_scalar);

SceneRaycastInfo raycast_scene(glm::vec3 ray_origin, glm::vec3 ray_direction, const Scene &scene);
SceneRaycastInfo raycast_scene(float mouse_position_x, float mouse_position_y, const PerspectiveCamera &camera,
                               const Scene &scene);

GizmoRaycastInfo raycast_gizmos(glm::vec3 ray_origin, glm::vec3 ray_direction, float gizmo_length,
                                const SceneObject &object);
GizmoRaycastInfo raycast_gizmos(float mouse_position_x, float mouse_position_y, float gizmo_length,
                                const PerspectiveCamera &camera, const SceneObject &object);

} // namespace ving
