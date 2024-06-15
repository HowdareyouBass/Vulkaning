#include "ving_gizmo_renderer.hpp"

#include <SDL3/SDL_log.h>

namespace ving
{
GizmoRenderer::GizmoRenderer(const Core &core)
{
    m_pipelines = core.create_graphics_render_pipelines<PushConstant>(
        "shaders/bin/gizmo.vert.spv", "shaders/bin/gizmo.frag.spv", {}, RenderFrames::render_image_format, {}, {}, {},
        vk::PrimitiveTopology::eLineList);
}

void GizmoRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera,
                           const SceneObject &target_object, editor::Gizmo::Type gizmo_type,
                           int32_t highlight_gizmo_index)
{
    m_push_constants.perspective_view_transform = camera.projection() * camera.view();
    m_push_constants.object_position = target_object.transform.translation;
    m_push_constants.gizmo_length = 0.5f;
    m_push_constants.highlight_gizmo_index = highlight_gizmo_index;

    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    start_rendering2d(cmd, img, vk::AttachmentLoadOp::eLoad);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    set_default_viewport_and_scissor(cmd, img);

    cmd.pushConstants<PushConstant>(m_pipelines.layout.get(),
                                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                    m_push_constants);

    cmd.draw(6, 1, 0, 0);

    cmd.endRendering();
}
} // namespace ving
