#include "ving_aabb.hpp"

namespace ving
{
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
