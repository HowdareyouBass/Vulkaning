#include "ving_aabb.hpp"
#include <algorithm>

namespace ving
{
void AABB::clamp()
{
    max.x = std::clamp(max.x, AABB::minimum_value, max.x);
    min.x = std::clamp(min.x, -AABB::minimum_value, min.x);
    max.y = std::clamp(max.y, AABB::minimum_value, max.y);
    min.y = std::clamp(min.y, -AABB::minimum_value, min.y);
    max.z = std::clamp(max.z, AABB::minimum_value, max.z);
    min.z = std::clamp(min.z, -AABB::minimum_value, min.z);
}

AABB AABB::to_world_space(const glm::mat4 &pvm_matrix, AABB aabb)
{
    // glm::vec4 new_min = pvm_matrix * glm::vec4{aabb.min, 1.0f};
    // aabb.min = new_min / new_min.w;
    // glm::vec4 new_max = pvm_matrix * glm::vec4{aabb.max, 1.0f};
    // aabb.max = new_max / new_max.w;
    glm::vec3 translation{pvm_matrix[3]};
    aabb.min += translation;
    aabb.max += translation;

    return aabb;
}
} // namespace ving
