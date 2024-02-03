#include "ving_engine.hpp"

#include <cmath>
#include <iostream>
#include <random>

#include <SDL3/SDL.h>

#include <imgui.h>

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

#include "ving_defaults.hpp"
#include "ving_descriptors.hpp"
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

    m_draw_image = Image2D{*m_device,
                           memory_properties,
                           vk::Extent3D{m_draw_extent, 1},
                           vk::Format::eR16G16B16A16Sfloat,
                           vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst |
                               vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment,
                           vk::ImageLayout::eUndefined};
    init_resources();
    init_descriptors();
    init_pipelines();
    init_imgui();
}
Engine::~Engine()
{
    ImGui_ImplVulkan_Shutdown();
}

void Engine::run()
{
    SDL_Event event;
    bool running = true;

    while (running)
    {
        auto start = std::chrono::high_resolution_clock::now();
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            ImGui_ImplSDL3_ProcessEvent(&event);

            // TODO: Stop rendering on window minimized
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Text("FPS: %.0f", (1.0f / m_delta_time) * 100.0f);
        ImGui::Text("Latency: %.2f ms", m_delta_time);

        ImGui::Render();

        draw();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> delta = end - start;
        m_delta_time = delta.count() * 100.0f;
        m_time += m_delta_time;
    }
    m_device->waitIdle();
}

void Engine::draw()
{
    // NOTE: Idk why but wait for fences is nodiscard however it checks result value in the function
    vk::Result result = m_device->waitForFences(*get_current_frame().render_fence, true, 1000000000);
    m_device->resetFences(*get_current_frame().render_fence);

    // HACK: Using uint64_t limit to wait for image because 1sec wasn't enough for first image acquisition
    auto acquire_image_res = m_device->acquireNextImageKHR(*m_swapchain, std::numeric_limits<uint64_t>::max(),
                                                           *get_current_frame().swapchain_semaphore);

    vk::resultCheck(acquire_image_res.result, "Failed to acquire swapchain image");
    uint32_t swapchain_image_index = acquire_image_res.value;

    vk::CommandBuffer cmd = *get_current_frame().command_buffer;

    cmd.reset();
    // Command Buffer Begin
    auto begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    vk::resultCheck(cmd.begin(&begin_info), "Command buffer begin failed");

    m_draw_image.transition_layout(cmd, vk::ImageLayout::eGeneral);

    draw_slime(cmd);

    utils::transition_image(cmd, m_swapchain_images[swapchain_image_index], vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eTransferDstOptimal);
    m_draw_image.transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal);

    utils::copy_image_to_image(cmd, m_draw_image.image(), m_swapchain_images[swapchain_image_index],
                               m_draw_image.extent(), m_swapchain_extent);
    utils::transition_image(cmd, m_swapchain_images[swapchain_image_index], vk::ImageLayout::eTransferDstOptimal,
                            vk::ImageLayout::eColorAttachmentOptimal);

    draw_imgui(cmd, *m_swapchain_image_views[swapchain_image_index]);

    utils::transition_image(cmd, m_swapchain_images[swapchain_image_index], vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ImageLayout::ePresentSrcKHR);
    cmd.end();
    // Command buffer end

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
void Engine::draw_imgui(vk::CommandBuffer cmd, vk::ImageView target_image_view)
{
    // auto color_attachment = def::color_attachment_render_info(target_image_view);
    //
    // auto render_info = def::rendering_info(m_swapchain_extent, color_attachment);
    //
    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                .setStoreOp(vk::AttachmentStoreOp::eStore)
                                .setLoadOp(vk::AttachmentLoadOp::eLoad)
                                .setImageView(target_image_view);

    auto render_info = vk::RenderingInfo{}
                           .setColorAttachments(color_attachment)
                           .setLayerCount(1)
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, m_swapchain_extent});

    cmd.beginRendering(render_info);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    cmd.endRendering();
}
void Engine::draw_slime(vk::CommandBuffer cmd)
{

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, *m_background_pipeline);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_background_layout, 0, m_background_descriptors, nullptr);

    SlimePushConstants push_constants;
    push_constants.delta_time = m_delta_time;
    push_constants.time = m_time;
    push_constants.dummy = 0;
    push_constants.agents_count = agent_count;
    cmd.pushConstants<SlimePushConstants>(*m_background_layout, vk::ShaderStageFlagBits::eCompute, 0, push_constants);

    cmd.dispatch(std::ceil(m_draw_image.extent().width / 16.0), std::ceil(m_draw_image.extent().height), 1);
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
            utils::create_swapchain_old(m_physical_device, *m_device, *m_surface, m_window_extent,
                                    graphics_family_index == present_family_index ? 1 : 2, frames_in_flight);
        m_swapchain = std::move(swapchain.first);
        m_swapchain_image_format = std::move(swapchain.second);
        m_swapchain_extent = m_window_extent;
        m_swapchain_images = m_device->getSwapchainImagesKHR(*m_swapchain);

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
    m_imm_pool = utils::create_command_pool(*m_device, m_queue_family_info.graphics_family,
                                            vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    m_imm_commands = std::move(utils::allocate_command_buffers(*m_device, *m_imm_pool, 1).back());
    m_imm_fence = utils::create_fence(*m_device, vk::FenceCreateFlagBits::eSignaled);
}
void Engine::init_resources()
{
    std::default_random_engine gen;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::uniform_int_distribution dist_pos(0, static_cast<int>(m_draw_extent.height));

    for (auto &&agent : m_agents)
    {
        agent.position = {dist_pos(gen), dist_pos(gen)};
        agent.angle = dist(gen);
    }

    m_agents_buffer = GPUBuffer{*m_device, memory_properties, m_agents.size() * sizeof(Agent),
                                vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                                vk::MemoryPropertyFlagBits::eDeviceLocal};
    GPUBuffer staging_buffer{*m_device, memory_properties, m_agents.size() * sizeof(Agent),
                             vk::BufferUsageFlagBits::eTransferSrc,
                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};
    staging_buffer.set_memory(*m_device, m_agents.data(), m_agents.size() * sizeof(Agent));
    staging_buffer.copy_to(*m_device, m_graphics_queue, *m_imm_pool, m_agents_buffer);
}
void Engine::init_descriptors()
{
    init_slime_descriptors();
}
void Engine::init_pipelines()
{
    init_slime_pipeline();
}
// TODO: Abstract this into a class
void Engine::init_imgui()
{
    vk::DescriptorPoolSize pool_sizes[] = {
        {vk::DescriptorType::eCombinedImageSampler, 1000}, {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},         {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},   {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000}, {vk::DescriptorType::eInputAttachment, 1000},
    };

    auto pool_info = vk::DescriptorPoolCreateInfo{}
                         .setMaxSets(1000)
                         .setPoolSizes(pool_sizes)
                         .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    m_imgui_pool = m_device->createDescriptorPoolUnique(pool_info);

    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplSDL3_InitForVulkan(m_window);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = *m_instance;
    init_info.PhysicalDevice = m_physical_device;
    init_info.Device = *m_device;
    init_info.Queue = m_graphics_queue;
    init_info.DescriptorPool = *m_imgui_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;
    init_info.ColorAttachmentFormat = static_cast<VkFormat>(m_swapchain_image_format);

    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, VK_NULL_HANDLE);

    ImGui_ImplVulkan_CreateFontsTexture();
    ImGui_ImplVulkan_DestroyFontsTexture();
}
void Engine::init_slime_descriptors()
{

    m_background_descriptor_layout = DescriptorLayoutBuilder{}
                                         .add_binding(0, vk::DescriptorType::eStorageImage)
                                         .add_binding(1, vk::DescriptorType::eStorageBuffer)
                                         .build(*m_device, vk::ShaderStageFlagBits::eCompute);

    auto sizes = std::vector<DescriptorAllocator::PoolSizeRatio>{
        {vk::DescriptorType::eStorageImage, 1},
        {vk::DescriptorType::eStorageBuffer, 1},
    };
    m_background_descriptor_allocator = DescriptorAllocator{*m_device, 10, sizes};

    m_background_descriptors = m_background_descriptor_allocator.allocate(*m_device, *m_background_descriptor_layout);

    DescriptorWriter writer{};

    writer.write_image(0, m_draw_image.view(), nullptr, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
    writer.write_buffer(1, m_agents_buffer.buffer(), m_agents_buffer.size(), 0, vk::DescriptorType::eStorageBuffer);

    for (auto &&descriptor : m_background_descriptors)
    {
        writer.update_set(*m_device, descriptor);
    }
}
void Engine::init_slime_pipeline()
{
    auto push_range =
        vk::PushConstantRange{}.setSize(sizeof(SlimePushConstants)).setStageFlags(vk::ShaderStageFlagBits::eCompute);
    auto layout_info =
        vk::PipelineLayoutCreateInfo{}.setSetLayouts(*m_background_descriptor_layout).setPushConstantRanges(push_range);
    m_background_layout = m_device->createPipelineLayoutUnique(layout_info);

    auto slime_shader = utils::create_shader_module(*m_device, "shaders/draw_slime.comp.spv");

    auto stage_info = vk::PipelineShaderStageCreateInfo{}
                          .setPName("main")
                          .setStage(vk::ShaderStageFlagBits::eCompute)
                          .setModule(*slime_shader);

    auto info = vk::ComputePipelineCreateInfo{}.setStage(stage_info).setLayout(*m_background_layout);

    m_background_pipeline = m_device->createComputePipelineUnique({}, info).value;
}
void Engine::immediate_submit(std::function<void(vk::CommandBuffer cmd)> &&function)
{
    m_device->resetFences(*m_imm_fence);
    m_imm_commands->reset();

    vk::CommandBuffer cmd = *m_imm_commands;

    auto cmd_begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    cmd.begin(cmd_begin_info);

    function(cmd);

    cmd.end();

    auto cmd_info = vk::CommandBufferSubmitInfo{}.setCommandBuffer(cmd);

    auto submit = vk::SubmitInfo2{}.setCommandBufferInfos(cmd_info);

    m_graphics_queue.submit2(submit, *m_imm_fence);

    vk::Result res = m_device->waitForFences(*m_imm_fence, true, std::numeric_limits<uint64_t>::max());
}
} // namespace ving
