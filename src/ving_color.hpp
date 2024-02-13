#pragma once

#include "glm/geometric.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace ving
{
static constexpr glm::vec4 blue{0.0f, 0.0f, 1.0f, 1.0f};
static constexpr glm::vec4 cyan{0.0f, 1.0f, 1.0f, 1.0f};
static const glm::vec4 slate_blue = {glm::normalize(glm::vec3{70.0f, 130.0f, 180.0f}), 1.0f};
} // namespace ving
