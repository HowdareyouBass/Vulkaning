#include "ving_utils.hpp"

#include <fstream>

#include <SDL3/SDL_vulkan.h>
#include <iostream>

#include "ving_defaults.hpp"

namespace ving
{
namespace utils
{
vk::UniqueInstance create_instance(std::string_view app_name, uint32_t app_version, std::string_view engine_name,
                                   uint32_t engine_version, std::span<const char *> layers,
                                   std::span<const char *> extensions)
{
    auto app_info = vk::ApplicationInfo{}
                        .setPApplicationName(app_name.data())
                        .setApplicationVersion(app_version)
                        .setPEngineName(engine_name.data())
                        .setEngineVersion(engine_version)
                        .setApiVersion(VK_API_VERSION_1_3);
    auto instance_create_info =
        vk::InstanceCreateInfo{}.setPApplicationInfo(&app_info).setPEnabledLayerNames(layers).setPEnabledExtensionNames(
            extensions);

    return vk::createInstanceUnique(instance_create_info);
}
vk::UniqueSurfaceKHR create_SDL_window_surface(SDL_Window *window, vk::Instance instance)
{

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), nullptr, &surface))
    {
        throw std::runtime_error(std::format("Failed to create vulkan surface{}", SDL_GetError()));
    }

    auto surface_deleter = vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>{instance};

    return vk::UniqueSurfaceKHR{surface, surface_deleter};
}
vk::PhysicalDevice pick_physical_device(std::span<vk::PhysicalDevice> devices)
{
    for (auto &&d : devices)
    {
        auto features2 = d.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                                        vk::PhysicalDeviceVulkan12Features>();

        const vk::PhysicalDeviceVulkan13Features &features13 = features2.get<vk::PhysicalDeviceVulkan13Features>();

        if (features13.dynamicRendering && features13.synchronization2)
        {
            return d;
        }
    }

    throw std::runtime_error("Could not find suitable Vulkan device");
}

// Return queue family index or queue_families.size() on fail
uint32_t find_queue_family(std::span<vk::QueueFamilyProperties> queue_families, vk::QueueFlags flags)
{
    auto queue_it = std::find_if(queue_families.begin(), queue_families.end(),
                                 [flags](vk::QueueFamilyProperties props) { return props.queueFlags | flags; });

    return std::distance(queue_families.begin(), queue_it);
}
uint32_t find_present_queue(std::span<vk::QueueFamilyProperties> queue_families, vk::PhysicalDevice physical_device,
                            vk::SurfaceKHR surface)
{
    uint32_t qsize = static_cast<uint32_t>(queue_families.size());

    for (uint32_t i = 0; i < qsize; ++i)
    {
        if (physical_device.getSurfaceSupportKHR(i, surface))
        {
            return i;
        }
    }
    return qsize;
}
vk::UniqueDevice create_device(vk::PhysicalDevice device, std::span<vk::DeviceQueueCreateInfo> queue_infos,
                               std::span<const char *> extensions, vk::PhysicalDeviceFeatures2 features2)
{
    auto info = vk::DeviceCreateInfo{}
                    .setPEnabledExtensionNames(extensions)
                    .setQueueCreateInfos(queue_infos)
                    .setPNext(&features2);

    return device.createDeviceUnique(info);
}
std::pair<vk::UniqueSwapchainKHR, vk::Format> create_swapchain_old(vk::PhysicalDevice physical_device,
                                                                   vk::Device device, vk::SurfaceKHR surface,
                                                                   vk::Extent2D extent, uint32_t queue_family_count,
                                                                   uint32_t image_count)
{
    std::vector<vk::SurfaceFormatKHR> formats = physical_device.getSurfaceFormatsKHR(surface);
    assert(!formats.empty());

    vk::Format format = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

    // HARD: Other present modes
    vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;

    if (image_count > surface_capabilities.maxImageCount || image_count < surface_capabilities.minImageCount)
    {
        throw std::runtime_error("Failed to create swapchain with required image count");
    }

    auto info =
        vk::SwapchainCreateInfoKHR{}
            .setSurface(surface)
            .setMinImageCount(image_count)
            .setImageFormat(format)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment)
            .setQueueFamilyIndexCount(queue_family_count)
            .setPreTransform(surface_capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(present_mode)
            .setImageSharingMode(queue_family_count > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);

    //.setImageSharingMode(sharing);

    return std::make_pair(device.createSwapchainKHRUnique(info), format);
}
vktypes::Swapchain create_swapchain(vk::PhysicalDevice physical_device, vk::Device device, vk::SurfaceKHR surface,
                                    vk::Extent2D extent, uint32_t queue_family_count, uint32_t image_count)
{
    vktypes::Swapchain new_swapchain;

    std::vector<vk::SurfaceFormatKHR> formats = physical_device.getSurfaceFormatsKHR(surface);
    assert(!formats.empty());

    new_swapchain.image_format =
        (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    vk::SurfaceCapabilitiesKHR surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

    // HARD: Other present modes
    vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;

    if (image_count > surface_capabilities.maxImageCount || image_count < surface_capabilities.minImageCount)
    {
        throw std::runtime_error("Failed to create swapchain with required image count");
    }

    auto info =
        vk::SwapchainCreateInfoKHR{}
            .setSurface(surface)
            .setMinImageCount(image_count)
            .setImageFormat(new_swapchain.image_format)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment)
            .setQueueFamilyIndexCount(queue_family_count)
            .setPreTransform(surface_capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(present_mode)
            .setImageSharingMode(queue_family_count > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);

    new_swapchain.swapchain = device.createSwapchainKHRUnique(info);
    new_swapchain.image_extent = extent;
    new_swapchain.image_layout = vk::ImageLayout::eUndefined;
    new_swapchain.images = device.getSwapchainImagesKHR(*new_swapchain.swapchain);
    new_swapchain.image_views = utils::create_image_views(device, new_swapchain.images, new_swapchain.image_format);

    return new_swapchain;
}
std::vector<vk::UniqueImageView> create_image_views(vk::Device device, std::span<vk::Image> images, vk::Format format)
{
    std::vector<vk::UniqueImageView> image_views{};
    image_views.reserve(images.size());

    auto info =
        vk::ImageViewCreateInfo{}
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(format)
            .setSubresourceRange(def::image_subresource_range_no_mip_no_levels(vk::ImageAspectFlagBits::eColor));

    for (auto &&image : images)
    {
        info.image = image;
        image_views.push_back(device.createImageViewUnique(info));
    }

    return image_views;
}
vk::UniqueCommandPool create_command_pool(vk::Device device, uint32_t queue_family, vk::CommandPoolCreateFlags flags)
{
    auto info = vk::CommandPoolCreateInfo{}.setQueueFamilyIndex(queue_family).setFlags(flags);

    return device.createCommandPoolUnique(info);
}
std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(vk::Device device, vk::CommandPool pool, uint32_t count)
{
    // HARD: Secondary command buffers
    auto info = vk::CommandBufferAllocateInfo{}
                    .setLevel(vk::CommandBufferLevel::ePrimary)
                    .setCommandPool(pool)
                    .setCommandBufferCount(count);

    return device.allocateCommandBuffersUnique(info);
}
vk::UniqueFence create_fence(vk::Device device, vk::FenceCreateFlags flags)
{
    auto info = vk::FenceCreateInfo{}.setFlags(flags);

    return device.createFenceUnique(info);
}
vk::UniqueSemaphore create_semaphore(vk::Device device)
{
    auto info = vk::SemaphoreCreateInfo{};

    return device.createSemaphoreUnique(info);
}
vk::UniqueShaderModule create_shader_module(vk::Device device, std::string_view shader_path)
{
    std::vector<uint32_t> shader_code = read_shader_file(shader_path);

    auto info = vk::ShaderModuleCreateInfo{}.setCode(shader_code);

    return device.createShaderModuleUnique(info);
}
void transition_image(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout current_layout,
                      vk::ImageLayout new_layout)
{
    vk::ImageAspectFlags aspect_mask = (new_layout == vk::ImageLayout::eDepthAttachmentOptimal)
                                           ? vk::ImageAspectFlagBits::eDepth
                                           : vk::ImageAspectFlagBits::eColor;

    auto image_barrier = vk::ImageMemoryBarrier2{}
                             .setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                             .setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite)
                             .setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                             .setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead)
                             .setOldLayout(current_layout)
                             .setNewLayout(new_layout)
                             .setSubresourceRange(vk::ImageSubresourceRange{}
                                                      .setAspectMask(aspect_mask)
                                                      .setBaseMipLevel(0)
                                                      .setLevelCount(vk::RemainingMipLevels)
                                                      .setBaseArrayLayer(0)
                                                      .setLayerCount(vk::RemainingArrayLayers))
                             .setImage(image);

    auto dep_info = vk::DependencyInfo{}.setImageMemoryBarriers(image_barrier);

    cmd.pipelineBarrier2(dep_info);
}
uint32_t find_memory_type(vk::PhysicalDeviceMemoryProperties mem_properties, uint32_t type_filter,
                          vk::MemoryPropertyFlags prop_flags)
{
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i)
    {
        if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & prop_flags) == prop_flags)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}
int get_format_size(vk::Format format)
{
    switch (format)
    {
    case vk::Format::eR8G8B8A8Unorm:
    case vk::Format::eD32Sfloat:
        return 4;
    case vk::Format::eR16G16B16A16Sfloat:
        return 8;
    default:
        return 0;
    }
}
std::vector<uint32_t> read_shader_file(std::filesystem::path path)
{
    std::ifstream file{path, std::ios::ate | std::ios::binary};

    // TODO: Proper error or excepiton handling
    if (!file.is_open())
    {
        std::cerr << "Failed to open the file: " << path;
        return {};
    }

    size_t file_size = file.tellg();

    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    file.seekg(0);

    file.read(reinterpret_cast<char *>(buffer.data()), file_size);

    file.close();

    return buffer;
}
void copy_image_to_image(vk::CommandBuffer cmd, vk::Image source, vk::Image dst, vk::Extent2D src_size,
                         vk::Extent2D dst_size)
{
    auto blit_region = vk::ImageBlit2{}
                           .setSrcOffsets({vk::Offset3D{}, vk::Offset3D{static_cast<int32_t>(src_size.width),
                                                                        static_cast<int32_t>(src_size.height), 1}})
                           .setDstOffsets({vk::Offset3D{}, vk::Offset3D{static_cast<int32_t>(dst_size.width),
                                                                        static_cast<int32_t>(dst_size.height), 1}})
                           .setSrcSubresource(vk::ImageSubresourceLayers{}
                                                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                                  .setBaseArrayLayer(0)
                                                  .setLayerCount(1)
                                                  .setMipLevel(0))
                           .setDstSubresource(vk::ImageSubresourceLayers{}
                                                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                                  .setBaseArrayLayer(0)
                                                  .setLayerCount(1)
                                                  .setMipLevel(0));

    auto blit_info = vk::BlitImageInfo2{}
                         .setFilter(vk::Filter::eLinear)
                         .setRegions(blit_region)
                         .setSrcImage(source)
                         .setDstImage(dst)
                         .setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal)
                         .setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);

    cmd.blitImage2(blit_info);
}
} // namespace utils
} // namespace ving
