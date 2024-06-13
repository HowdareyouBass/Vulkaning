#include "ving_scene_object.hpp"
#include "ving_color.hpp"
#include "ving_core.hpp"
#include "ving_logger.hpp"
#include "ving_vertex.hpp"

#include <iostream>

namespace ving
{

RayTracedMesh SimpleMesh::cube_raytraced(const Core &core, vk::TransformMatrixKHR &transform,
                                         std::array<glm::vec4, 8> vertex_colors)
{
    // clang-format off
    std::array<uint32_t, 36> indices{
        0,4,5,5,1,0,
        2,6,7,7,3,2,
        1,5,6,6,2,1,
        0,3,7,7,4,0,
        4,7,6,6,5,4,
        0,1,2,2,3,0,
    };
    // clang-format on

    std::array<Vertex, 8> vertices{
        // Bottom
        Vertex{{-1.0f, -1.0f, 1.0f}, 0.0f, {0.0f, -1.0f, 0.0f}, 0.0f, vertex_colors[0]},
        Vertex{{1.0f, -1.0f, 1.0f}, 1.0f, {0.0f, -1.0f, 0.0f}, 0.0f, vertex_colors[1]},
        Vertex{{1.0f, -1.0f, -1.0f}, 1.0f, {0.0f, -1.0f, 0.0f}, 1.0f, vertex_colors[2]},
        Vertex{{-1.0f, -1.0f, -1.0f}, 0.0f, {0.0f, -1.0f, 0.0f}, 1.0f, vertex_colors[3]},
        // Top
        Vertex{{-1.0f, 1.0f, 1.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 1.0f, vertex_colors[4]},
        Vertex{{1.0f, 1.0f, 1.0f}, 1.0f, {0.0f, 1.0f, 0.0f}, 1.0f, vertex_colors[5]},
        Vertex{{1.0f, 1.0f, -1.0f}, 1.0f, {0.0f, 1.0f, 0.0f}, 0.0f, vertex_colors[6]},
        Vertex{{-1.0f, 1.0f, -1.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f, vertex_colors[7]},
    };

    GPUBuffer vertex_buffer =
        core.create_gpu_buffer(vertices.data(), sizeof(Vertex) * vertices.size(),
                               vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                   vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
    GPUBuffer index_buffer =
        core.create_gpu_buffer(indices.data(), sizeof(uint32_t) * indices.size(),
                               vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                   vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
    GPUBuffer transform_buffer =
        core.create_gpu_buffer(&transform, sizeof(transform),
                               vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                   vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

    return {std::move(vertex_buffer), std::move(index_buffer), std::move(transform_buffer), vertices.size()};
}
glm::mat4 Transform::mat4() const noexcept
{
    glm::mat4 mtx{1.0f};

    // mtx[1][1] = -mtx[1][1];

    mtx = glm::translate(mtx, translation);
    mtx = glm::scale(mtx, scale);
    mtx = glm::rotate(mtx, rotation.y, {0.0f, 1.0f, 0.0f});
    mtx = glm::rotate(mtx, rotation.x, {1.0f, 0.0f, 0.0f});
    mtx = glm::rotate(mtx, rotation.z, {0.0f, 0.0f, 1.0f});

    return mtx;
}
AABB SceneObject::get_world_space_aabb()
{
    return AABB::to_world_space(transform.mat4(), mesh.aabb);
}
} // namespace ving
