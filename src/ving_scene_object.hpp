#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "ving_gpu_buffer.hpp"

namespace ving
{
struct Vertex
{
    glm::vec3 position;
    float uv_x;
    glm::vec3 normal;
    float uv_y;
    glm::vec4 color;
};

struct GPUMeshBuffers
{
    GPUBuffer index_buffer;
    GPUBuffer vertex_buffer;
    vk::DeviceAddress vertex_buffer_address;
};

struct Mesh
{
    GPUMeshBuffers gpu_buffers;
    uint32_t indices_count;
    uint32_t vertices_count;
};

class Core;
struct SimpleMesh
{
    // Vulkan space i = {1, 0, 0} j = {0, -1, 0} k = {0, 0, -1}
    static Mesh flat_plane(const Core &core, uint32_t length, uint32_t width, float spacing, glm::vec4 color);
    static Mesh quad(const Core &core, glm::vec4 color);
    // Engine space i = {1, 0, 0} j = {0, 1, 0} k = {0, 0, 1}
    static Mesh cube_interpolated_normals(const Core &core, std::array<glm::vec4, 8> vertex_colors = {});
};

struct Transform
{
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 translation{0.0f};

    [[nodiscard]] glm::mat4 mat4() const noexcept
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);

        return glm::mat4{{
                             scale.x * (c1 * c3 + s1 * s2 * s3),
                             scale.x * (c2 * s3),
                             scale.x * (c1 * s2 * s3 - c3 * s1),
                             0.0f,
                         },
                         {
                             scale.y * (c3 * s1 * s2 - c1 * s3),
                             scale.y * (c2 * c3),
                             scale.y * (c1 * c3 * s2 + s1 * s3),
                             0.0f,
                         },
                         {
                             scale.z * (c2 * s1),
                             scale.z * (-s2),
                             scale.z * (c1 * c2),
                             0.0f,
                         },
                         {translation.x, translation.y, translation.z, 1.0f}};
    }
};

struct SceneObject
{
    Mesh mesh;
    Transform transform;
};
} // namespace ving
