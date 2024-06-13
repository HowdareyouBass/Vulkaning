#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
} // namespace ving
