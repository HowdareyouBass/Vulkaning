#pragma once

#include <functional>

#include <vulkan/vulkan.hpp>

#include "ving_base_renderer.hpp"
#include "ving_descriptors.hpp"
#include "ving_gpu_buffer.hpp"
#include "ving_image.hpp"
#include "ving_utils.hpp"
#include "vk_types.hpp"

struct SDL_Window;
struct ImGui_ImplVulkan_InitInfo;

namespace ving
{
struct GPUMeshBuffers;
struct Vertex;
class Core
{
    struct QueueFamiliesInfo
    {
        uint32_t graphics_family;
        uint32_t present_family;
        uint32_t transfer_family;

        bool is_graphics_and_present_same() const { return graphics_family == present_family; }
    };

  public:
    struct DescriptorBinding
    {
        uint32_t binding;
        vk::DescriptorType type;
    };
    // HARD: Let the user enable or disable layers
    static constexpr bool enable_validation_layers = true;

    Core(SDL_Window *window);

    ImGui_ImplVulkan_InitInfo create_imgui_init_info(vk::DescriptorPool pool, vk::Format color_attachment_format) const;

    Image2D create_image2d(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage,
                           vk::ImageLayout layout) const;
    GPUBuffer create_gpu_buffer(void *data, uint64_t size, vk::BufferUsageFlags usage) const;
    vktypes::Swapchain create_swapchain(uint32_t image_count) const;
    vk::UniqueSemaphore create_semaphore() const;
    vk::UniqueFence create_fence(bool state) const;

    GPUMeshBuffers allocate_gpu_mesh_buffers(std::span<uint32_t> indices, std::span<Vertex> vertices) const;
    std::vector<vk::UniqueCommandBuffer> allocate_command_buffers(uint32_t count) const;
    BaseRenderer::RenderResources allocate_render_resources(std::span<BaseRenderer::RenderResourceCreateInfo> infos,
                                                            vk::ShaderStageFlags stage) const;

    void wait_for_fence(vk::Fence fence) const
    {
        vk::resultCheck(m_device->waitForFences(fence, true, 1000000000), "Wait for fences failed");
    }
    void wait_idle() const { m_device->waitIdle(); }
    void reset_fence(vk::Fence fence) const { m_device->resetFences(fence); }
    uint32_t acquire_swapchain_image(vk::SwapchainKHR swapchain, vk::Semaphore semaphore) const
    {
        return m_device->acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), semaphore).value;
    }

    vk::Queue get_present_queue() const noexcept { return m_device->getQueue(m_queue_info.present_family, 0); }
    vk::Queue get_graphics_queue() const noexcept { return m_device->getQueue(m_queue_info.graphics_family, 0); }
    vk::Extent2D get_window_extent() const noexcept { return m_window_extent; }
    vk::CommandPool get_command_pool() const noexcept { return *m_command_pool; }
    vk::Device device() const noexcept { return *m_device; }

    // TODO: More descriptor layouts??
    template <typename PushConstantsType>
    BaseRenderer::Pipelines create_compute_render_pipelines(vk::DescriptorSetLayout descriptor_layout,
                                                            std::string_view shader_path) const
    {
        auto push_range =
            vk::PushConstantRange{}.setSize(sizeof(PushConstantsType)).setStageFlags(vk::ShaderStageFlagBits::eCompute);
        auto layout_info =
            vk::PipelineLayoutCreateInfo{}.setSetLayouts(descriptor_layout).setPushConstantRanges(push_range);
        auto layout = m_device->createPipelineLayoutUnique(layout_info);

        auto shader = utils::create_shader_module(*m_device, shader_path);

        auto stage_info = vk::PipelineShaderStageCreateInfo{}
                              .setPName("main")
                              .setStage(vk::ShaderStageFlagBits::eCompute)
                              .setModule(*shader);

        auto info = vk::ComputePipelineCreateInfo{}.setStage(stage_info).setLayout(*layout);

        return {m_device->createComputePipelineUnique({}, info).value, std::move(layout)};
    }
    // HARD: Most of the settings are hardcoded
    template <typename PushConstantsType>
    BaseRenderer::Pipelines create_graphics_render_pipelines(std::string_view vertex_shader_path,
                                                             std::string_view fragment_shader_path,
                                                             vk::DescriptorSetLayout descriptor_layout,
                                                             vk::Format color_attachment_format,
                                                             vk::Format depth_attachment_format) const
    {
        auto fragment_shader = utils::create_shader_module(*m_device, fragment_shader_path);
        auto vertex_shader = utils::create_shader_module(*m_device, vertex_shader_path);
        // HARD: Shader stages should be defined by user
        auto push_range = vk::PushConstantRange{}
                              .setOffset(0)
                              .setSize(sizeof(PushConstantsType))
                              .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
        auto layout_info =
            vk::PipelineLayoutCreateInfo{}.setPushConstantRanges(push_range).setSetLayouts(descriptor_layout);

        auto layout = m_device->createPipelineLayoutUnique(layout_info);

        auto shader_stages = std::vector<vk::PipelineShaderStageCreateInfo>{
            vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eVertex)
                .setModule(*vertex_shader)
                .setPName("main"),

            vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eFragment)
                .setModule(*fragment_shader)
                .setPName("main"),
        };

        auto input_assembly =
            vk::PipelineInputAssemblyStateCreateInfo{}.setTopology(vk::PrimitiveTopology::eTriangleList);
        auto vertex_input = vk::PipelineVertexInputStateCreateInfo{};
        auto viewport_state = vk::PipelineViewportStateCreateInfo{}.setViewportCount(1).setScissorCount(1);
        auto rasterizer = vk::PipelineRasterizationStateCreateInfo{}
                              .setPolygonMode(vk::PolygonMode::eFill)
                              .setLineWidth(1.0f)
                              .setCullMode(vk::CullModeFlagBits::eNone)
                              .setFrontFace(vk::FrontFace::eClockwise);
        auto multisampling = vk::PipelineMultisampleStateCreateInfo{}
                                 .setSampleShadingEnable(vk::False)
                                 .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                                 .setMinSampleShading(1.0f)
                                 .setAlphaToCoverageEnable(vk::False)
                                 .setAlphaToOneEnable(vk::False);
        // HARD: Shouuld be controled
        auto color_blend_attachment =
            vk::PipelineColorBlendAttachmentState{}
                .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                   vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                .setBlendEnable(vk::False);

        auto color_blend = vk::PipelineColorBlendStateCreateInfo{}
                               .setLogicOpEnable(vk::False)
                               .setLogicOp(vk::LogicOp::eCopy)
                               .setAttachments(color_blend_attachment);

        auto depthtest = vk::PipelineDepthStencilStateCreateInfo{}
                             .setDepthTestEnable(vk::True)
                             .setDepthWriteEnable(vk::True)
                             .setDepthCompareOp(vk::CompareOp::eGreaterOrEqual)
                             .setDepthBoundsTestEnable(vk::False)
                             .setStencilTestEnable(vk::False)
                             .setMinDepthBounds(0.0f)
                             .setMaxDepthBounds(1.0f);

        auto render_info = vk::PipelineRenderingCreateInfo{}
                               .setColorAttachmentFormats(color_attachment_format)
                               .setDepthAttachmentFormat(depth_attachment_format);

        vk::DynamicState states[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        auto dynamic_info = vk::PipelineDynamicStateCreateInfo{}.setDynamicStates(states);

        auto pipeline_info = vk::GraphicsPipelineCreateInfo{}
                                 .setStages(shader_stages)
                                 .setPVertexInputState(&vertex_input)
                                 .setPViewportState(&viewport_state)
                                 .setPInputAssemblyState(&input_assembly)
                                 .setPRasterizationState(&rasterizer)
                                 .setPMultisampleState(&multisampling)
                                 .setPColorBlendState(&color_blend)
                                 .setPDepthStencilState(&depthtest)
                                 .setPDynamicState(&dynamic_info)
                                 .setLayout(*layout)
                                 .setPNext(&render_info);

        auto pipeline_res = m_device->createGraphicsPipelineUnique({}, pipeline_info);
        vk::resultCheck(pipeline_res.result, "Failed to create graphics pipeline");

        return BaseRenderer::Pipelines{std::move(pipeline_res.value), std::move(layout)};
    }

  public:
    vk::PhysicalDeviceMemoryProperties memory_properties;

  private:
    void immediate_transfer(std::function<void(vk::CommandBuffer)> &&function) const;

    std::vector<const char *> m_required_instance_layers{};
    std::vector<const char *> m_required_instance_extensions{};
    std::vector<const char *> m_required_device_extensions{VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                           VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::Extent2D m_window_extent;

    vk::UniqueInstance m_instance;
    vk::PhysicalDevice m_physical_device;

    vk::UniqueSurfaceKHR m_surface;

    QueueFamiliesInfo m_queue_info;
    vk::UniqueDevice m_device;

    vk::UniqueCommandPool m_command_pool;

    // TODO: abstract this into transfer queue class
    vk::Queue m_transfer_queue;
    vk::UniqueCommandPool m_transfer_pool;
    vk::UniqueCommandBuffer m_transfer_commands;
    vk::UniqueFence m_transfer_fence;
};
} // namespace ving
