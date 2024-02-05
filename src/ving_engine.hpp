#pragma once

#include <functional>

#define VULKAN_HPP_NO_TO_STRING
#include <vulkan/vulkan.hpp>

#include <SDL3/SDL_vulkan.h>
#include <glm/vec2.hpp>

#include "ving_descriptors.hpp"
#include "ving_gpu_buffer.hpp"
#include "ving_image.hpp"

namespace ving
{
struct SlimePushConstants
{
    float delta_time;
    float time;
    float dummy;
    int agents_count;
};

struct Agent
{
    glm::vec2 position;
    float angle;
    float dummy;
};

class [[deprecated]] Engine
{
  public:
    struct FrameData
    {
        vk::UniqueCommandPool command_pool;
        vk::UniqueCommandBuffer command_buffer;

        vk::UniqueSemaphore swapchain_semaphore, render_semaphore;
        vk::UniqueFence render_fence;
    };
    struct QueueFamilyInfo
    {
        uint32_t graphics_family;
        uint32_t present_family;
    };

    Engine();
    ~Engine();

    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
    Engine(Engine &&) = delete;
    Engine &operator=(Engine &&) = delete;

    // HARD: More frames in flight (tripple buffering)
    static constexpr int frames_in_flight = 2;
    static constexpr bool enable_validation_layers = true;

    static constexpr int start_window_width = 1280;
    static constexpr int start_window_height = 720;

    // Slime
    // FIXME: This gives 14fps at 500000 guys :(
    static constexpr int agent_count = 100000;

    // TODO: Maybe abstract this into device class
    vk::PhysicalDeviceMemoryProperties memory_properties;

    std::vector<const char *> required_layers{};
    std::vector<const char *> required_instance_extensions{};
    std::vector<const char *> required_device_extensions{VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                                                         VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  public:
    void run();

    FrameData &get_current_frame() { return m_frames[m_frame_number % frames_in_flight]; }

  private:
    void draw();
    void draw_imgui(vk::CommandBuffer cmd, vk::ImageView target_image_view);
    void draw_slime(vk::CommandBuffer cmd);

    void init_window();
    void init_vulkan();
    void init_frames();
    void init_resources();
    void init_descriptors();
    void init_pipelines();
    void init_imgui();
    void init_slime_descriptors();
    void init_slime_pipeline();

    // HACK: Could use another queue to submit
    void immediate_submit(std::function<void(vk::CommandBuffer cmd)> &&function);

  private:
    float m_delta_time{0};
    float m_time{0};

    vk::Extent2D m_window_extent{start_window_width, start_window_height};

    SDL_Window *m_window;

    vk::UniqueInstance m_instance;
    vk::UniqueSurfaceKHR m_surface;
    vk::PhysicalDevice m_physical_device;
    vk::UniqueDevice m_device;

    QueueFamilyInfo m_queue_family_info;

    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;

    vk::UniqueSwapchainKHR m_swapchain;
    vk::Format m_swapchain_image_format;
    vk::Extent2D m_swapchain_extent;
    std::vector<vk::Image> m_swapchain_images;
    std::vector<vk::UniqueImageView> m_swapchain_image_views;

    uint32_t m_frame_number{0};
    std::array<FrameData, frames_in_flight> m_frames;

    Image2D m_draw_image;
    vk::Extent2D m_draw_extent;

    // Immidiate submit structures
    vk::UniqueFence m_imm_fence;
    vk::UniqueCommandPool m_imm_pool;
    vk::UniqueCommandBuffer m_imm_commands;

    // Slime Project
    vk::UniqueDescriptorSetLayout m_background_descriptor_layout;
    DescriptorAllocator m_background_descriptor_allocator;
    std::vector<vk::DescriptorSet> m_background_descriptors;
    vk::UniquePipelineLayout m_background_layout;
    vk::UniquePipeline m_background_pipeline;

    std::array<Agent, agent_count> m_agents;
    GPUBuffer m_agents_buffer;

    // Imgui stuff
    vk::UniqueDescriptorPool m_imgui_pool;
};
} // namespace ving
