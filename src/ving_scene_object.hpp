#pragma once

#include <vulkan/vulkan.hpp>

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

class SceneObject
{
};
} // namespace ving
