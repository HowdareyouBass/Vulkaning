#include "ving_ray.hpp"

#include <SDL3/SDL_log.h>

namespace ving
{
struct RayHit
{
    bool hit;
    float t{std::numeric_limits<float>::max()};
};
RaycastInfo raycast_plane(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 plane_normal,
                          float plane_normal_scalar)
{
    float delim = glm::dot(plane_normal, ray_direction);

    if (delim > glm::epsilon<float>())
    {
        float normal_length_sq = glm::dot(plane_normal, plane_normal);
        float t = (plane_normal_scalar * normal_length_sq - glm::dot(plane_normal, ray_origin)) / delim;
        return {true, {ray_origin + ray_direction * t}};
    }
    return {false, {}};
}

RaycastInfo raycast_plane(float mouse_position_x, float mouse_position_y, const PerspectiveCamera &camera,
                          glm::vec3 plane_normal, float plane_normal_scalar)
{
    glm::vec3 ray_dir = glm::normalize(mouse_position_x * camera.aspect() * camera.right() +
                                       mouse_position_y * (1.0f / camera.aspect()) * camera.up() +
                                       camera.forward() * (1.0f / glm::tan(camera.fov() / 2.0f)));

    return raycast_plane(camera.position, ray_dir, plane_normal, plane_normal_scalar);
}

// Took this algorithm from https://tavinator.com/2022/ray_box_boundary.html
RayHit raycast_aabb(glm::vec3 ray_origin, glm::vec3 ray_direction, const AABB &aabb)
{
    glm::vec3 ray_direction_inverse = 1.0f / ray_direction;

    float tmin = 0.0f, tmax = std::numeric_limits<float>::infinity();
    float t1, t2;

    for (int d = 0; d < 3; ++d)
    {
        t1 = (aabb.min[d] - ray_origin[d]) * ray_direction_inverse[d];
        t2 = (aabb.max[d] - ray_origin[d]) * ray_direction_inverse[d];

        tmin = std::min(std::max(t1, tmin), std::max(t2, tmin));
        tmax = std::max(std::min(t1, tmax), std::min(t2, tmax));
    }
    return {tmin <= tmax, tmin};
}

// TODO: use batching and sse instructions
SceneRaycastInfo raycast_scene(glm::vec3 ray_origin, glm::vec3 ray_direction, const Scene &scene)
{
    bool hit = false;

    struct
    {
        uint32_t id{0};
        float t{std::numeric_limits<float>::infinity()};
    } closest_hit;

    for (size_t i = 0; i < scene.objects.size(); ++i)
    {
        AABB aabb_world_space = AABB::to_world_space(scene.objects[i].transform.mat4(), scene.objects[i].mesh.aabb);

        RayHit aabb_hit = raycast_aabb(ray_origin, ray_direction, aabb_world_space);

        if (aabb_hit.hit && aabb_hit.t < closest_hit.t)
        {
            hit = true;
            closest_hit.id = i;
            closest_hit.t = aabb_hit.t;
        }
    }
    return {hit, closest_hit.id, ray_origin + closest_hit.t * ray_direction};
}
// TODO: Merge with raycast_scene
GizmoRaycastInfo raycast_gizmos(glm::vec3 ray_origin, glm::vec3 ray_direction, const SceneObject &object)
{
    bool hit = false;
    editor::Gizmo::Type type;

    std::array<AABB, 3> gizmo_aabbs = editor::Gizmo::make_gizmo_aabbs(object);

    for (uint32_t i = 0; i < 3; ++i)
    {
        AABB aabb_world_space = AABB::to_world_space(object.transform.mat4(), gizmo_aabbs[i]);
        RayHit aabb_hit = raycast_aabb(ray_origin, ray_direction, aabb_world_space);

        if (aabb_hit.hit)
        {
            hit = true;
            type = static_cast<editor::Gizmo::Type>(i);
            break;
        }
    }

    return {hit, type};
}
GizmoRaycastInfo raycast_gizmos(float mouse_position_x, float mouse_position_y, const PerspectiveCamera &camera,
                                const SceneObject &object)
{
    glm::vec3 ray_dir = glm::normalize(mouse_position_x * camera.aspect() * camera.right() +
                                       mouse_position_y * (1.0f / camera.aspect()) * camera.up() +
                                       camera.forward() * (1.0f / glm::tan(camera.fov() / 2.0f)));

    return raycast_gizmos(camera.position, ray_dir, object);
}
SceneRaycastInfo raycast_scene(float mouse_position_x, float mouse_position_y, const PerspectiveCamera &camera,
                               const Scene &scene)
{
    glm::vec3 ray_dir = glm::normalize(mouse_position_x * camera.aspect() * camera.right() +
                                       mouse_position_y * (1.0f / camera.aspect()) * camera.up() +
                                       camera.forward() * (1.0f / glm::tan(camera.fov() / 2.0f)));

    return raycast_scene(camera.position, ray_dir, scene);
}
} // namespace ving
