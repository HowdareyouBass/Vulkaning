#include "ving_aabb_generator.hpp"

namespace ving
{
AABBGenerator::AABBGenerator(const Core &core) : r_core{core}
{
    m_resources = core.allocate_render_resources(m_render_resource_infos, vk::ShaderStageFlagBits::eCompute);

    m_generate_pipeline = core.create_compute_render_pipelines<PushConstants>(m_resources.layouts(),
                                                                              "shaders/bin/aabb_generate.comp.spv");
}

// TODO: Generate with compute
void AABBGenerator::generate(RenderFrames &frames, Scene &scene)
{
    // WARN: Very bad optimize this
    // Allocating buffer for every frame is very bad
    m_generated_aabbs = r_core.create_cpu_visible_gpu_buffer(sizeof(AABB) * scene.objects.size(),
                                                             vk::BufferUsageFlagBits::eStorageBuffer);
    m_resources.get_resource(0).write_buffer(r_core.device(), 0, m_generated_aabbs);

    frames.immediate_submit([&scene, this](vk::CommandBuffer cmd) {
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, m_generate_pipeline.pipeline.get());

        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_generate_pipeline.layout.get(), 0,
                               m_resources.descriptors(), {});

        for (uint32_t i = 0; auto &&obj : scene.objects)
        {
            m_push_constants.vertex_buffer_address = obj.mesh.gpu_buffers.vertex_buffer_address;
            m_push_constants.object_index = i;
            cmd.pushConstants<PushConstants>(m_generate_pipeline.layout.get(), vk::ShaderStageFlagBits::eCompute, 0,
                                             m_push_constants);
            cmd.dispatch(std::ceil(obj.mesh.vertices_count / 32.0), 1, 1);
            ++i;
        }
    });

    m_generated_aabbs.map_data();
    scene.aabbs = std::span<AABB>{static_cast<AABB *>(m_generated_aabbs.data()),
                                  static_cast<AABB *>(m_generated_aabbs.data()) + scene.objects.size()};
    // m_generated_aabbs.unmap_data();
}
} // namespace ving
