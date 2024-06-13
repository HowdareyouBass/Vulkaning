#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "ving_aabb.hpp"
#include "ving_gpu_buffer.hpp"
#include "ving_mesh.hpp"

namespace ving
{
class Core;

struct RayTracedMesh
{
    GPUBuffer vertex_buffer;
    GPUBuffer index_buffer;
    GPUBuffer transform_buffer;
    uint32_t vertices_count;
};

class Core;
struct SimpleMesh
{
    static RayTracedMesh cube_raytraced(const Core &core, vk::TransformMatrixKHR &transform,
                                        std::array<glm::vec4, 8> vertex_colors = {});
};

struct Transform
{
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 translation{0.0f};

    [[nodiscard]] glm::mat4 mat4() const noexcept;
};

struct SceneObject
{
    const Mesh &mesh;
    Transform transform;

    AABB get_world_space_aabb();
};
} // namespace ving
