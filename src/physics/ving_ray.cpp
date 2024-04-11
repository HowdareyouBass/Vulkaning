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

std::pair<bool, RayHitInfo> raycast_scene(glm::vec3 origin, glm::vec3 direction, const Scene &scene)
{
    for (size_t i = 0; i < scene.objects.size(); ++i)
    {
        const AABB &aabb = scene.objects[i].mesh.aabb;
        const float *max = &aabb.max_x;
        const float *min = max + 3;

        Quadrant quadrant[3];

        bool inside = true;
        glm::vec3 candidate_plane;

        for (int i = 0; i < 3; ++i)
        {
            if (origin[i] < min[i])
            {
                quadrant[i] = Quadrant::Left;
                candidate_plane[i] = min[i];
                inside = false;
            }
            else if (origin[i] > max[i])
            {
                quadrant[i] = Quadrant::Right;
                candidate_plane[i] = max[i];
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

        return {true, {i, hit_pos}};
    }
    return {false, {}};
}
} // namespace ving
