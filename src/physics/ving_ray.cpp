#include "ving_ray.hpp"

#include <SDL3/SDL_log.h>

namespace ving
{
struct RayHit
{
    bool hit;
    float t{std::numeric_limits<float>::max()};
};

// Took this algorithm from https://tavinator.com/2022/ray_box_boundary.html
RayHit raycast_aabb(glm::vec3 origin, glm::vec3 direction, const AABB &aabb)
{
    glm::vec3 direction_inverse = 1.0f / direction;

    float tmin = 0.0f, tmax = std::numeric_limits<float>::infinity();
    float t1, t2;

    for (int d = 0; d < 3; ++d)
    {
        t1 = (aabb.min[d] - origin[d]) * direction_inverse[d];
        t2 = (aabb.max[d] - origin[d]) * direction_inverse[d];

        tmin = std::min(std::max(t1, tmin), std::max(t2, tmin));
        tmax = std::max(std::min(t1, tmax), std::min(t2, tmax));
    }
    return {tmin <= tmax, tmin};
}

// TODO: use batching and sse instructions
std::pair<bool, RayHitInfo> raycast_scene(glm::vec3 origin, glm::vec3 direction, const Scene &scene)
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

        RayHit aabb_hit = raycast_aabb(origin, direction, aabb_world_space);

        if (aabb_hit.hit && aabb_hit.t < closest_hit.t)
        {
            hit = true;
            closest_hit.id = i;
            closest_hit.t = aabb_hit.t;
        }
    }
    return {hit, RayHitInfo{closest_hit.id, origin + closest_hit.t * direction}};
}
// TODO: Merge with raycast_scene
std::pair<bool, editor::Gizmo::Type> raycast_gizmos(glm::vec3 origin, glm::vec3 direction, const SceneObject &object)
{
    bool hit = false;
    editor::Gizmo::Type type;

    constexpr float gizmo_aabb_offset = 0.05f;

    glm::vec3 object_position = object.transform.translation;
    AABB gizmo_aabbs[3]{
        // X
        {{0.0f, -gizmo_aabb_offset, -gizmo_aabb_offset}, {editor::Gizmo::length, gizmo_aabb_offset, gizmo_aabb_offset}},
        // Y
        {{-gizmo_aabb_offset, 0.0f, -gizmo_aabb_offset}, {gizmo_aabb_offset, editor::Gizmo::length, gizmo_aabb_offset}},
        // Z
        {{-gizmo_aabb_offset, -gizmo_aabb_offset, 0.0f}, {gizmo_aabb_offset, gizmo_aabb_offset, editor::Gizmo::length}},
    };

    for (uint32_t i = 0; i < 3; ++i)
    {
        AABB aabb_world_space = AABB::to_world_space(object.transform.mat4(), gizmo_aabbs[i]);
        RayHit aabb_hit = raycast_aabb(origin, direction, aabb_world_space);

        if (aabb_hit.hit)
        {
            hit = true;
            type = static_cast<editor::Gizmo::Type>(i);
            break;
        }
    }

    return {hit, type};
}
std::pair<bool, editor::Gizmo::Type> raycast_gizmos(float mouse_position_x, float mouse_position_y,
                                                    const PerspectiveCamera &camera, const SceneObject &object)
{
#if 0
    glm::vec4 origin{0.0f, 0.0f, 0.0f, 1.0f};

    origin = origin * camera.projection() * camera.view();

    glm::vec4 direction{0.0f};

    return raycast_gizmos(origin, direction, object);
#endif

    // FIXME: It's not tan of fov
    return raycast_gizmos(camera.position,
                          glm::normalize(glm::tan(camera.fov() / 2.0f) * camera.forward() +
                                         mouse_position_x * camera.right() + mouse_position_y * camera.up()),
                          object);
}
std::pair<bool, RayHitInfo> raycast_scene(float mouse_position_x, float mouse_position_y,
                                          const PerspectiveCamera &camera, const Scene &scene)
{
    return raycast_scene(camera.position,
                         glm::normalize(glm::tan(camera.fov() / 2.0f) * camera.forward() +
                                        mouse_position_x * camera.right() + mouse_position_y * camera.up()),
                         scene);
}

// https://github.com/erich666/GraphicsGems/blob/master/gems/RayBox.c
//
enum class Quadrant
{
    Left,
    Right,
    Middle
};
std::pair<bool, RayHitInfo> raycast_scene_old(glm::vec3 origin, glm::vec3 direction, const Scene &scene)
{
    // direction.y *= -1;
    origin.y *= -1;

    for (size_t i = 0; i < scene.objects.size(); ++i)
    {
        const AABB &aabb = scene.objects[i].mesh.aabb;

        Quadrant quadrant[3];

        bool inside = true;
        glm::vec3 candidate_plane;

        for (int i = 0; i < 3; ++i)
        {
            if (origin[i] < aabb.min[i])
            {
                quadrant[i] = Quadrant::Left;
                candidate_plane[i] = aabb.min[i];
                inside = false;
            }
            else if (origin[i] > aabb.max[i])
            {
                quadrant[i] = Quadrant::Right;
                candidate_plane[i] = aabb.max[i];
                inside = false;
            }
            else
            {
                quadrant[i] = Quadrant::Middle;
            }
        }

        if (inside)
        {
            return {true, RayHitInfo{i, origin}};
        }

        glm::vec3 max_t{-1.0f};
        for (int i = 0; i < 3; ++i)
        {
            if (quadrant[i] != Quadrant::Middle && direction[i] != 0.0f)
                max_t[i] = (candidate_plane[i] - origin[i]) / direction[i];
        }

        int which_plane = 0;
        for (int i = 1; i < 3; ++i)
        {
            if (max_t[which_plane] < max_t[i])
                which_plane = i;
        }

        if (max_t[which_plane] < 0.0f)
            return {false, {}};

        glm::vec3 hit_pos = origin + max_t[which_plane] * direction;

        if (hit_pos.x < aabb.min.x || hit_pos.x > aabb.max.x || hit_pos.y < aabb.min.y || hit_pos.y > aabb.max.y ||
            hit_pos.z < aabb.min.z || hit_pos.z > aabb.max.z)
        {
            continue;
        }

        return {true, {i, hit_pos}};
    }
    return {false, {}};
}
} // namespace ving
