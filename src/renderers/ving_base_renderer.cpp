#include "ving_base_renderer.hpp"
#include "ving_image.hpp"

namespace ving
{
void BaseRenderer::start_rendering2d(vk::CommandBuffer cmd, const Image2D &render_image,
                                     vk::AttachmentLoadOp attachment_load_op) const
{
    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(render_image.view())
                                .setImageLayout(render_image.layout())
                                .setLoadOp(attachment_load_op)
                                .setStoreOp(vk::AttachmentStoreOp::eStore)
                                .setClearValue(clear);

    auto render_info = vk::RenderingInfo{}
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, render_image.extent()})
                           .setLayerCount(1)
                           .setColorAttachments(color_attachment);

    cmd.beginRendering(render_info);
}
void BaseRenderer::start_rendering3d(vk::CommandBuffer cmd, const Image2D &render_image, const Image2D &depth_image,
                                     vk::AttachmentLoadOp load_op) const
{
    assert(render_image.extent() == depth_image.extent());

    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(render_image.view())
                                .setImageLayout(render_image.layout())
                                .setLoadOp(load_op)
                                .setStoreOp(vk::AttachmentStoreOp::eStore)
                                .setClearValue(clear);

    auto depth_clear = vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{}.setDepth(0.0f));

    auto depth_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(depth_image.view())
                                .setImageLayout(depth_image.layout())
                                .setLoadOp(vk::AttachmentLoadOp::eClear)
                                .setStoreOp(vk::AttachmentStoreOp::eStore)
                                .setClearValue(depth_clear);

    auto render_info = vk::RenderingInfo{}
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, render_image.extent()})
                           .setLayerCount(1)
                           .setColorAttachments(color_attachment)
                           .setPDepthAttachment(&depth_attachment);

    cmd.beginRendering(render_info);
}
void BaseRenderer::set_default_viewport_and_scissor(vk::CommandBuffer cmd, const Image2D &img) const
{
    auto viewport =
        vk::Viewport{}.setWidth(img.extent().width).setHeight(img.extent().height).setMinDepth(0.0f).setMaxDepth(1.0f);
    auto scissor = vk::Rect2D{}.setOffset({0, 0}).setExtent(img.extent());
    cmd.setViewport(0, viewport);
    cmd.setScissor(0, scissor);
}
} // namespace ving
