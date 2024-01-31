#include "ving_engine.hpp"

#include <cmath>
#include <iostream>

#include <SDL3/SDL.h>

#include "ving_defaults.hpp"
#include "ving_utils.hpp"

namespace ving
{
Engine::Engine()
{
    if constexpr (enable_validation_layers)
    {
        required_layers.push_back("VK_LAYER_KHRONOS_validation");
        required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    init_window();
    init_vulkan();
    init_frames();

    m_draw_extent = m_window_extent;

    m_draw_image =
        Image2D{*m_device, memory_properties, vk::Extent3D{m_draw_extent, 1}, vk::Format::eR16G16B16A16Sfloat,
                vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst |
                    vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment};
}
Engine::~Engine()
{
    m_device->waitIdle();
}

void Engine::run()
{
    SDL_Event event;
    bool running = true;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            // TODO: Stop rendering on window minimized
        }
        draw();
    }
}

void Engine::draw()
{
    // NOTE: Idk why but wait for fences is nodiscard however it checks result value in the function
    vk::Result result = m_device->waitForFences(*get_current_frame().render_fence, true, 1000000000);
    m_device->resetFences(*get_current_frame().render_fence);

    // HACK: Using uint64_t limit to wait for image because 1sec wasn't enought for first image acquisition
    auto acqurie_image_res = m_device->acquireNextImageKHR(*m_swapchain, std::numeric_limits<uint64_t>::max(),
                                                           *get_current_frame().swapchain_semaphore);

    vk::resultCheck(acqurie_image_res.result, "Failed to acquire swapchain image");

    uint32_t swapchain_image_index = acqurie_image_res.value;

    vk::CommandBuffer cmd = *get_current_frame().command_buffer;

    cmd.reset();

    auto begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    vk::resultCheck(cmd.begin(&begin_info), "Command buffer begin failed");

    utils::transition_image(cmd, m_swapchain_images[swapchain_image_index], vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eGeneral);

    float flash = std::abs(std::sin(m_frame_number / 120.0f));
    auto clear_value = vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, flash, 1.0f});
    auto clear_range = def::image_subresource_range_no_mip_no_levels(vk::ImageAspectFlagBits::eColor);

    cmd.clearColorImage(m_swapchain_images[swapchain_image_index], vk::ImageLayout::eGeneral, clear_value, clear_range);

    utils::transition_image(cmd, m_swapchain_images[swapchain_image_index], vk::ImageLayout::eGeneral,
                            vk::ImageLayout::ePresentSrcKHR);

    cmd.end();

    auto cmd_info = vk::CommandBufferSubmitInfo{}.setCommandBuffer(cmd);

    auto wait_info = vk::SemaphoreSubmitInfo{}
                         .setSemaphore(*get_current_frame().swapchain_semaphore)
                         .setStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                         .setValue(1);

    auto signal_info = vk::SemaphoreSubmitInfo{}
                           .setSemaphore(*get_current_frame().render_semaphore)
                           .setStageMask(vk::PipelineStageFlagBits2::eAllCommands)
                           .setValue(1);

    auto submit = vk::SubmitInfo2{}
                      .setWaitSemaphoreInfos(wait_info)
                      .setSignalSemaphoreInfos(signal_info)
                      .setCommandBufferInfos(cmd_info);

    m_graphics_queue.submit2(submit, *get_current_frame().render_fence);

    auto present_info = vk::PresentInfoKHR{}
                            .setSwapchains(*m_swapchain)
                            .setWaitSemaphores(*get_current_frame().render_semaphore)
                            .setImageIndices(swapchain_image_index);

    vk::Result present_res = m_present_queue.presentKHR(&present_info);

    m_frame_number++;
}
void Engine::init_window()
{
    if (SDL_Init(SDL_InitFlags::SDL_INIT_VIDEO) < 0)
    {
        throw std::runtime_error(std::format("Couldn't initialize SDL: {}", SDL_GetError()));
    }

    m_window = SDL_CreateWindow("Vulkaning", m_window_extent.width, m_window_extent.height, SDL_WINDOW_VULKAN);

    if (!m_window)
    {
        throw std::runtime_error(std::format("Failed to create SDL window: {}", SDL_GetError()));
    }

    uint32_t count = 0;
    const char *const *window_extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    required_instance_extensions.reserve(required_instance_extensions.size() + count);
    for (uint32_t i = 0; i < count; ++i)
    {
        required_instance_extensions.push_back(window_extensions[i]);
    }
}
void Engine::init_vulkan()
{
    m_instance = utils::create_instance("None", VK_MAKE_VERSION(0, 0, 1), "Vulkaning", VK_MAKE_VERSION(0, 0, 1),
                                        required_layers, required_instance_extensions);

    m_surface = utils::create_SDL_window_surface(m_window, *m_instance);

    std::vector<vk::PhysicalDevice> available_devices = m_instance->enumeratePhysicalDevices();
    m_physical_device = utils::pick_physical_device(available_devices);

    memory_properties = m_physical_device.getMemoryProperties();

    std::vector<vk::QueueFamilyProperties> queue_families = m_physical_device.getQueueFamilyProperties();
    uint32_t graphics_family_index = utils::find_queue_family(queue_families, vk::QueueFlagBits::eGraphics);
    if (graphics_family_index == queue_families.size())
    {
        throw std::runtime_error("Failed to find graphics queue on current device");
    }
    uint32_t present_family_index = utils::find_present_queue(queue_families, m_physical_device, *m_surface);
    if (present_family_index == queue_families.size())
    {
        throw std::runtime_error("Failed to find present queue on current device");
    }

    m_queue_family_info.present_family = present_family_index;
    m_queue_family_info.graphics_family = graphics_family_index;

    // HARD: Hardcoded
    float queue_priority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> queue_infos = {
        vk::DeviceQueueCreateInfo{}
            .setQueueCount(1)
            .setQueuePriorities(queue_priority)
            .setQueueFamilyIndex(graphics_family_index),
    };

    if (graphics_family_index != present_family_index)
    {
        queue_infos.push_back(vk::DeviceQueueCreateInfo{}
                                  .setQueueCount(1)
                                  .setQueuePriorities(queue_priority)
                                  .setQueueFamilyIndex(present_family_index));
    }

    auto features2 = m_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                                                    vk::PhysicalDeviceVulkan12Features>();
    m_device = utils::create_device(m_physical_device, queue_infos, required_device_extensions,
                                    features2.get<vk::PhysicalDeviceFeatures2>());

    // WARN: Could actually be the same queue
    m_graphics_queue = m_device->getQueue(graphics_family_index, 0);
    m_present_queue = m_device->getQueue(present_family_index, 0);

    {
        auto swapchain =
            utils::create_swapchain(m_physical_device, *m_device, *m_surface, m_window_extent,
                                    graphics_family_index == present_family_index ? 1 : 2, frames_in_flight);
        m_swapchain = std::move(swapchain.first);
        m_swapchain_image_format = std::move(swapchain.second);

        m_swapchain_images = m_device->getSwapchainImagesKHR(*m_swapchain);
#if 0
        m_swapchain_image_views.reserve(m_swapchain_images.size());
        auto image_view_info =
            vk::ImageViewCreateInfo{}
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(m_swapchain_image_format)
                .setSubresourceRange(def::image_subresource_range_no_mip_no_levels(vk::ImageAspectFlagBits::eColor));
        for (auto &&image : m_swapchain_images)
        {
            image_view_info.image = image;
            m_swapchain_image_views.push_back(m_device->createImageViewUnique(image_view_info));
        }
#endif
    }
}
void Engine::init_frames()
{
    for (auto &&frame : m_frames)
    {
        frame.command_pool = utils::create_command_pool(*m_device, m_queue_family_info.graphics_family,
                                                        vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        std::vector<vk::UniqueCommandBuffer> buffers =
            utils::allocate_command_buffers(*m_device, *frame.command_pool, 1);

        frame.command_buffer = std::move(buffers[0]);
        frame.render_fence = utils::create_fence(*m_device, vk::FenceCreateFlagBits::eSignaled);
        frame.swapchain_semaphore = utils::create_semaphore(*m_device);
        frame.render_semaphore = utils::create_semaphore(*m_device);
    }
}
} // namespace ving
