#pragma once

#include "ving_core.hpp"
#include "ving_render_frames.hpp"
#include "ving_scene.hpp"

namespace ving
{
class AABBGenerator
{
    struct PushConstants
    {
        vk::DeviceAddress vertex_buffer_address;
        uint32_t object_index;
    };

  public:
    AABBGenerator(const Core &core);

    void generate(RenderFrames &frames, Scene &scene);

  private:
    const Core &r_core;

    std::vector<RenderResourceCreateInfo> m_render_resource_infos{
        {0,
         {
             {0, vk::DescriptorType::eStorageBuffer}, // AABBs buffer
         }},
    };

    RenderResources m_resources;
    Core::Pipelines m_generate_pipeline;

    PushConstants m_push_constants;

    GPUBuffer m_generated_aabbs;
};
} // namespace ving
