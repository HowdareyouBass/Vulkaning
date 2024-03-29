#include "ving_imgui_renderer.hpp"
#include "ving_profilers.hpp"

#include <SDL3/SDL_mouse.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

namespace ving
{

ImGuiRenderer::ImGuiRenderer(const Core &core, SDL_Window *window)
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

    m_pool = core.device().createDescriptorPoolUnique(pool_info);

    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForVulkan(window);

    // HARD: Should do render target class to get information about draw_image
    ImGui_ImplVulkan_InitInfo init_info = core.create_imgui_init_info(*m_pool, vk::Format::eR16G16B16A16Sfloat);

    ImGui_ImplVulkan_Init(&init_info, VK_NULL_HANDLE);

    ImGui_ImplVulkan_CreateFontsTexture();
    ImGui_ImplVulkan_DestroyFontsTexture();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
}

ImGuiRenderer::~ImGuiRenderer()
{
    ImGui_ImplVulkan_Shutdown();
}
void ImGuiRenderer::render(const RenderFrames::FrameInfo &frame, Profiler &profiler,
                           const std::vector<std::function<void()>> &imgui_frames)
{
    auto imgui_render = profiler.start_scoped_task("ImGui Render");

    ImGui::NewFrame();

    ImGui::Begin("Profile");
    ImGui::Text("FPS: %.0f", (1.0f / frame.delta_time) * 1000.0f);
    ImGui::Text("Latency: %.2f ms", frame.delta_time);
    // ImGui::Text("Mouse");
    // float mouse_x, mouse_y;
    // SDL_GetMouseState(&mouse_x, &mouse_y);
    // ImGui::Text("x:%f y:%f", mouse_x, mouse_y);
    profiler.imgui_frame();
    profiler.flush();
    ImGui::End();

    ImGui::Begin("Render Data Controls");
    for (auto &&frame : imgui_frames)
    {
        frame();
    }
    ImGui::End();

    // ImGui::ShowDemoWindow();

    ImGui::EndFrame();

    ImGui::Render();

    vk::CommandBuffer cmd = frame.cmd;

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                .setLoadOp(vk::AttachmentLoadOp::eLoad)
                                .setStoreOp(vk::AttachmentStoreOp::eStore)
                                .setImageView(frame.draw_image.view());

    auto render_info = vk::RenderingInfo{}
                           .setColorAttachments(color_attachment)
                           .setLayerCount(1)
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, frame.draw_image.extent()});

    frame.draw_image.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);
    cmd.beginRendering(render_info);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    cmd.endRendering();
}

void ImGuiRenderer::process_sdl_event(const SDL_Event &event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
}
} // namespace ving
