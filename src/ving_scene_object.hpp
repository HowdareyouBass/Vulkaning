#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "ving_gpu_buffer.hpp"

namespace ving
{
class Core;

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

    static Mesh load_from_file(const Core &core, std::string_view filepath);
};

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
    // Vulkan space i = {1, 0, 0} j = {0, -1, 0} k = {0, 0, -1}
    static Mesh flat_plane(const Core &core, uint32_t length, uint32_t width, float spacing, glm::vec4 color);
    static Mesh quad(const Core &core, glm::vec4 color);
    // Engine space i = {1, 0, 0} j = {0, 1, 0} k = {0, 0, 1}
    static Mesh cube_interpolated_normals(const Core &core, std::array<glm::vec4, 8> vertex_colors = {});

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
    Mesh mesh;
    Transform transform;
};
} // namespace ving
