#include "ving_base_renderer.hpp"
#include "ving_image.hpp"

namespace ving
{
void BaseRenderer::start_rendering2d(vk::CommandBuffer cmd, const Image2D &render_image) const
{
    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(render_image.view())
                                .setImageLayout(render_image.layout())
                                .setLoadOp(vk::AttachmentLoadOp::eClear)
                                .setStoreOp(vk::AttachmentStoreOp::eStore)
                                .setClearValue(clear);

    auto render_info = vk::RenderingInfo{}
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, render_image.extent()})
                           .setLayerCount(1)
                           .setColorAttachments(color_attachment);

    cmd.beginRendering(render_info);
}
} // namespace ving
