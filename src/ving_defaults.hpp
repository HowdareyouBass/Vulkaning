#pragma once

namespace ving
{
namespace def
{
vk::ImageSubresourceRange image_subresource_range_no_mip_no_levels(vk::ImageAspectFlags aspect_mask);
vk::RenderingAttachmentInfo color_attachment_render_info(vk::ImageView view,
                                                         std::optional<vk::ClearValue> clear_value = {});
vk::RenderingInfo rendering_info(vk::Extent2D render_extent, vk::RenderingAttachmentInfo color_attachment,
                                 std::optional<vk::RenderingAttachmentInfo> depth_attachment = {});
} // namespace def
} // namespace ving
