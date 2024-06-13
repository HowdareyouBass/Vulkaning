#pragma once

#include <filesystem>

#include "ving_aabb.hpp"
#include "ving_gpu_buffer.hpp"

namespace ving
{

class Core;

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

    AABB aabb;

    static Mesh load_from_file(const Core &core, std::filesystem::path filepath,
                               glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f});

    static Mesh flat_plane(const Core &core, uint32_t length, uint32_t width, float spacing, glm::vec4 color);
    static Mesh quad(const Core &core, glm::vec4 color);
    static Mesh cube_interpolated_normals(const Core &core, std::array<glm::vec4, 8> vertex_colors = {});
    static Mesh sphere(const Core &core, float radius, glm::vec4 color);
};
} // namespace ving
