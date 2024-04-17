#include "ving_ray.hpp"

namespace ving
{
// https://github.com/erich666/GraphicsGems/blob/master/gems/RayBox.c
//
enum class Quadrant
{
    Left,
    Right,
    Middle
};

// Took this algorithm from https://tavinator.com/2022/ray_box_boundary.html
// TODO: maybe use batching and sse instructions
std::pair<bool, RayHitInfo> raycast_scene(glm::vec3 origin, glm::vec3 direction, const Scene &scene)
{

    glm::vec3 direction_inverse = 1.0f / direction;

    for (size_t i = 0; i < scene.objects.size(); ++i)
    {
        // TODO: Maybe precalc the world space aabbs in the mesh constructing
        AABB aabb_world_space = scene.objects[i].mesh.aabb;

        glm::vec4 new_min = scene.objects[i].transform.mat4() * glm::vec4{aabb_world_space.min, 1.0f};
        aabb_world_space.min = new_min / new_min.w;

        glm::vec4 new_max = scene.objects[i].transform.mat4() * glm::vec4{aabb_world_space.max, 1.0f};
        aabb_world_space.max = new_max / new_max.w;

        float tmin = 0.0f, tmax = std::numeric_limits<float>::infinity();
        float t1, t2;

        for (int d = 0; d < 3; ++d)
        {
            t1 = (aabb_world_space.min[d] - origin[d]) * direction_inverse[d];
            t2 = (aabb_world_space.max[d] - origin[d]) * direction_inverse[d];

            tmin = std::min(std::max(t1, tmin), std::max(t2, tmin));
            tmax = std::max(std::min(t1, tmax), std::min(t2, tmax));
        }

        if (tmin <= tmax)
            return {true, RayHitInfo{i, origin + direction * tmin}};
    }
    return {false, {}};
}
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
