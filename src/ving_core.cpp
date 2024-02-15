#include "ving_core.hpp"

#include <SDL3/SDL_vulkan.h>
#include <backends/imgui_impl_vulkan.h>

#include "ving_descriptors.hpp"
#include "ving_scene_object.hpp"
#include "ving_utils.hpp"

namespace ving
{
Core::Core(SDL_Window *window)
{
    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);
    m_window_extent = vk::Extent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    if constexpr (enable_validation_layers)
    {
        m_required_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
        m_required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    uint32_t count = 0;
    const char *const *window_extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    m_required_instance_extensions.reserve(m_required_instance_extensions.size() + count);

    for (uint32_t i = 0; i < count; ++i)
    {
        m_required_instance_extensions.push_back(window_extensions[i]);
    }

    m_instance = utils::create_instance("None", VK_MAKE_VERSION(0, 0, 1), "Vulkaning", VK_MAKE_VERSION(0, 0, 2),
                                        m_required_instance_layers, m_required_instance_extensions);

    std::vector<vk::PhysicalDevice> available_devices = m_instance->enumeratePhysicalDevices();
    m_physical_device = utils::pick_physical_device(available_devices);
    memory_properties = m_physical_device.getMemoryProperties();

    m_surface = utils::create_SDL_window_surface(window, *m_instance);

    std::vector<vk::QueueFamilyProperties> queue_families = m_physical_device.getQueueFamilyProperties();

    m_queue_info.graphics_family = utils::find_queue_family(queue_families, vk::QueueFlagBits::eGraphics);
    if (m_queue_info.graphics_family == queue_families.size())
        throw std::runtime_error("Failed to find graphics queue on current device");

    if (m_physical_device.getSurfaceSupportKHR(m_queue_info.graphics_family, *m_surface))
    {
        m_queue_info.present_family = m_queue_info.graphics_family;
    }
    else
    {
        m_queue_info.present_family = utils::find_present_queue(queue_families, m_physical_device, *m_surface);
        if (m_queue_info.present_family == queue_families.size())
            throw std::runtime_error("Failed to find present queue on current device");
    }

    m_queue_info.transfer_family = utils::find_queue_family(queue_families, vk::QueueFlagBits::eTransfer);

    if (m_queue_info.transfer_family == queue_families.size())
        throw std::runtime_error("Failed to find transfer queue on current device");

    // HARD: Graphics should have more priority??

    float queue_priority = 1.0f;

    std::vector<uint32_t> queue_families_set{m_queue_info.graphics_family, m_queue_info.present_family,
                                             m_queue_info.transfer_family};
    queue_families_set.erase(std::unique(queue_families_set.begin(), queue_families_set.end()),
                             queue_families_set.end());
    std::vector<vk::DeviceQueueCreateInfo> device_queue_infos{};

    for (auto &&queue_family : queue_families_set)
    {
        device_queue_infos.push_back(vk::DeviceQueueCreateInfo{}
                                         .setQueueCount(1)
                                         .setQueuePriorities(queue_priority)
                                         .setQueueFamilyIndex(queue_family));
    }

    auto features2 = m_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                                                    vk::PhysicalDeviceVulkan12Features>();

    m_device = utils::create_device(m_physical_device, device_queue_infos, m_required_device_extensions,
                                    features2.get<vk::PhysicalDeviceFeatures2>());

    m_command_pool = utils::create_command_pool(*m_device, m_queue_info.graphics_family,
                                                vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    m_transfer_queue = m_device->getQueue(m_queue_info.transfer_family, 0);

    m_transfer_pool = utils::create_command_pool(*m_device, m_queue_info.transfer_family,
                                                 vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    m_transfer_commands = std::move(utils::allocate_command_buffers(*m_device, *m_transfer_pool, 1)[0]);
    m_transfer_fence = utils::create_fence(*m_device, vk::FenceCreateFlagBits::eSignaled);
}
ImGui_ImplVulkan_InitInfo Core::create_imgui_init_info(vk::DescriptorPool pool,
                                                       vk::Format color_attachment_format) const
{
    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = *m_instance;
    info.PhysicalDevice = m_physical_device;
    info.Device = *m_device;
    info.Queue = get_graphics_queue();
    info.DescriptorPool = pool;
    info.MinImageCount = 3;
    info.ImageCount = 3;
    info.UseDynamicRendering = true;
    info.ColorAttachmentFormat = static_cast<VkFormat>(color_attachment_format);

    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    return info;
}
Image2D Core::create_image2d(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage,
                             vk::ImageLayout layout) const
{
    return Image2D{*m_device, memory_properties, size, format, usage, layout};
}
// NOTE: Allocates only GPU Memory!!
GPUBuffer Core::create_gpu_buffer(void *data, uint64_t size, vk::BufferUsageFlags usage) const
{
    GPUBuffer new_buffer{*m_device, memory_properties, size, usage | vk::BufferUsageFlagBits::eTransferDst,
                         vk::MemoryPropertyFlagBits::eDeviceLocal};
    GPUBuffer staging{*m_device, memory_properties, size, vk::BufferUsageFlagBits::eTransferSrc,
                      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};
    staging.set_memory(*m_device, data, size);
    immediate_transfer([&](vk::CommandBuffer cmd) {
        auto copy = vk::BufferCopy{}.setSize(size);
        cmd.copyBuffer(staging.buffer(), new_buffer.buffer(), copy);
    });

    return new_buffer;
}
vktypes::Swapchain Core::create_swapchain(uint32_t image_count) const
{
    return utils::create_swapchain(m_physical_device, *m_device, *m_surface, m_window_extent,
                                   m_queue_info.is_graphics_and_present_same() ? 1 : 2, image_count);
}
vk::UniqueSemaphore Core::create_semaphore() const
{
    return utils::create_semaphore(*m_device);
}
vk::UniqueFence Core::create_fence(bool state) const
{
    auto info = vk::FenceCreateInfo{};

    if (state)
        info.setFlags(vk::FenceCreateFlagBits::eSignaled);

    return m_device->createFenceUnique(info);
}
GPUMeshBuffers Core::allocate_gpu_mesh_buffers(std::span<uint32_t> indices, std::span<Vertex> vertices) const
{
    GPUBuffer index_buffer =
        create_gpu_buffer(indices.data(), sizeof(uint32_t) * indices.size(), vk::BufferUsageFlagBits::eIndexBuffer);

    // NOTE: Using Buffer usage shader device address bit so shader can access the buffer
    GPUBuffer vertex_buffer =
        create_gpu_buffer(vertices.data(), sizeof(Vertex) * vertices.size(),
                          vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);
    auto buffer_address_info = vk::BufferDeviceAddressInfo{}.setBuffer(vertex_buffer.buffer());

    vk::DeviceAddress vertex_buffer_address = m_device->getBufferAddress(buffer_address_info);

    return {std::move(index_buffer), std::move(vertex_buffer), vertex_buffer_address};
}
std::vector<vk::UniqueCommandBuffer> Core::allocate_command_buffers(uint32_t count) const
{
    return utils::allocate_command_buffers(*m_device, *m_command_pool, count);
}

RenderResources Core::allocate_render_resources(std::span<RenderResourceCreateInfo> infos,
                                                vk::ShaderStageFlags stage) const
{
    return RenderResources{m_device.get(), infos, stage};
}
void Core::immediate_transfer(std::function<void(vk::CommandBuffer)> &&function) const
{
    m_device->resetFences(*m_transfer_fence);
    m_transfer_commands->reset();

    const vk::CommandBuffer &cmd = *m_transfer_commands;

    auto cmd_begin_info = vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd.begin(cmd_begin_info);
    function(cmd);
    cmd.end();

    auto cmd_info = vk::CommandBufferSubmitInfo{}.setCommandBuffer(cmd);

    auto submit = vk::SubmitInfo2{}.setCommandBufferInfos(cmd_info);

    m_transfer_queue.submit2(submit, *m_transfer_fence);

    vk::resultCheck(m_device->waitForFences(*m_transfer_fence, true, std::numeric_limits<uint64_t>::max()),
                    "Failed to wait for immediate transfer");
}
} // namespace ving
