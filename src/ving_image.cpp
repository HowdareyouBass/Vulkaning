#include "ving_image.hpp"

#include "ving_defaults.hpp"
#include "ving_utils.hpp"

namespace ving
{
Image2D::Image2D(vk::Device device, vk::PhysicalDeviceMemoryProperties device_mem_props, vk::Extent3D extent,
                 vk::Format format, vk::ImageUsageFlags usage)
{
    // Image
    auto info = vk::ImageCreateInfo{}
                    .setImageType(vk::ImageType::e2D)
                    .setFormat(format)
                    .setExtent(extent)
                    .setMipLevels(1)
                    .setArrayLayers(1)
                    .setUsage(usage);
    VkImageCreateInfo cinfo = static_cast<VkImageCreateInfo>(info);

    uint32_t bytes_per_pixel = utils::get_format_size(format);

    m_image = device.createImageUnique(info);
    // Memory
    auto image_memory_requirements = device.getImageMemoryRequirements(*m_image);
    auto alloc_info =
        vk::MemoryAllocateInfo{}
            .setAllocationSize(extent.width * extent.height * extent.depth * bytes_per_pixel)
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

    auto view_info = vk::ImageViewCreateInfo{}
                         .setImage(*m_image)
                         .setViewType(vk::ImageViewType::e2D)
                         .setFormat(format)
                         .setSubresourceRange(def::image_subresource_range_no_mip_no_levels(aspect_flags));

    m_view = device.createImageViewUnique(view_info);
    m_layout = vk::ImageLayout::eUndefined;
    m_format = format;
    m_extent = extent;
}

void Image2D::transition_layout(vk::CommandBuffer cmd, vk::ImageLayout new_layout)
{
    utils::transition_image(cmd, *m_image, m_layout, new_layout);

    m_layout = new_layout;
}
} // namespace ving
