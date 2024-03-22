#pragma once

#include "glm/geometric.hpp"
#include "ving_image.hpp"
#include "ving_scene_object.hpp"
#include <functional>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <imgui.h>

namespace ving
{
struct Scene
{
    // NOTE: W for light intencity
    glm::vec4 light_direction{glm::normalize(glm::vec3{0.4f, 0.6f, 0.23f}), 1.5f};

    Image2D skybox_cubemap;
    vk::UniqueSampler skybox_sampler;

    std::vector<SceneObject> objects;
    std::span<AABB> aabbs;

    std::function<void()> get_imgui()
    {
        return [this]() {
            ImGui::DragFloat3("Light Direction", reinterpret_cast<float *>(&light_direction), 0.01f, -1.0f, 1.0f);
        };
    }
};
} // namespace ving
