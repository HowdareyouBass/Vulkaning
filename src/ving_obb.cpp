#include "ving_obb.hpp"

namespace ving
{
OBB::OBB(AABB aabb, glm::mat4 model_mtx)
{
    center = glm::vec3{model_mtx[3]};
    normals = {
        glm::vec3{model_mtx[0]},
        glm::vec3{model_mtx[1]},
        glm::vec3{model_mtx[2]},
    };
    h = (aabb.max - aabb.min) / 2.0f;
}
} // namespace ving
