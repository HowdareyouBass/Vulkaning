#include "ving_image.hpp"

#include "ving_utils.hpp"

namespace ving
{
Image2D::Image2D(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, vk::Extent3D extent,
                 vk::Format format, vk::ImageUsageFlags usage, vk::ImageLayout layout, uint32_t mip_levels_count,
                 uint32_t layers_count, vk::ImageCreateFlags flags, vk::UniqueSampler sampler)
{
    // Image
    auto info = vk::ImageCreateInfo{}
                    .setImageType(vk::ImageType::e2D)
                    .setFormat(format)
                    .setExtent(extent)
                    .setUsage(usage)
                    .setInitialLayout(layout)
                    .setMipLevels(mip_levels_count)
                    .setArrayLayers(layers_count)
                    .setFlags(flags);

    m_image = device.createImageUnique(info);
    // Memory
    // uint32_t bytes_per_pixel = utils::get_format_size(format);
    auto image_memory_requirements = device.getImageMemoryRequirements(*m_image);
    auto alloc_info =
        vk::MemoryAllocateInfo{}
            .setAllocationSize(image_memory_requirements.size)
            .setMemoryTypeIndex(utils::find_memory_type(device_mem_props, image_memory_requirements.memoryTypeBits,
                                                        vk::MemoryPropertyFlagBits::eDeviceLocal));
    m_memory = device.allocateMemoryUnique(alloc_info);
    device.bindImageMemory(*m_image, *m_memory, 0);

    // Image view
    // HARD: Other aspectflags
    vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eColor;
    if (format == vk::Format::eD32Sfloat)
    {
        aspect_flags = vk::ImageAspectFlagBits::eDepth;
    }

    auto subresource_range = vk::ImageSubresourceRange{}
                                 .setAspectMask(aspect_flags)
                                 .setLevelCount(mip_levels_count)
                                 .setLayerCount(layers_count);

    auto view_info =
        vk::ImageViewCreateInfo{}.setImage(*m_image).setFormat(format).setSubresourceRange(subresource_range);

    // HARD: Temporary
    if (flags & vk::ImageCreateFlagBits::eCubeCompatible)
    {
        view_info.viewType = vk::ImageViewType::eCube;
    }
    else
    {
        view_info.viewType = vk::ImageViewType::e2D;
    }

    m_view = device.createImageViewUnique(view_info);
    m_layout = layout;
    m_format = format;
    m_extent = extent;
    m_mip_count = mip_levels_count;
    m_layer_count = layers_count;
    m_sampler = std::move(sampler);
}

void Image2D::transition_layout(vk::CommandBuffer cmd, vk::ImageLayout new_layout)
{
    if (new_layout == m_layout)
        return;

    vk::ImageAspectFlags aspect_mask = (new_layout == vk::ImageLayout::eDepthAttachmentOptimal)
                                           ? vk::ImageAspectFlagBits::eDepth
                                           : vk::ImageAspectFlagBits::eColor;

    auto image_barrier = vk::ImageMemoryBarrier2{}
                             .setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                             .setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
                             .setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                             .setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead)
                             .setOldLayout(m_layout)
                             .setNewLayout(new_layout)
                             .setSubresourceRange(vk::ImageSubresourceRange{}
                                                      .setAspectMask(aspect_mask)
                                                      .setLevelCount(m_mip_count)
                                                      .setLayerCount(m_layer_count))
                             .setImage(m_image.get());

    auto dep_info = vk::DependencyInfo{}.setImageMemoryBarriers(image_barrier);

    cmd.pipelineBarrier2(dep_info);

    m_layout = new_layout;
}
void Image2D::copy_to(vk::CommandBuffer cmd, const Image2D &image) const
{
    utils::copy_image_to_image(cmd, m_image.get(), image.image(), vk::Extent2D{m_extent.width, m_extent.height},
                               vk::Extent2D{image.extent().width, image.extent().height});
}
} // namespace ving
