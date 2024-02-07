#include "ving_simple_cube_renderer.hpp"

#include <glm/gtc/constants.hpp>

namespace ving
{
SimpleCubeRenderer::SimpleCubeRenderer(const Core &core)
{
    m_depth_img = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eD32Sfloat,
                                      vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eUndefined);

    auto resource_info = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{vk::DescriptorType::eUniformBuffer, 0},
    };

    m_resources = core.allocate_render_resources(resource_info,
                                                 vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);

    std::vector<uint32_t> cube_indices{0, 2, 1, 0, 3, 2, 0, 5, 6, 1, 6, 2, 0, 5, 1,
                                       0, 4, 5, 0, 4, 7, 0, 7, 3, 5, 7, 6, 4, 7, 5};
    std::vector<Vertex> cube_vertices{
        Vertex{glm::vec3{-0.5f, 0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, 0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, 0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}},
        Vertex{glm::vec3{-0.5f, 0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{-0.5f, -0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, -0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, -0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{-0.5f, -0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}},
    };

    m_cube_mesh = core.allocate_mesh(cube_indices, cube_vertices);
    m_push_constants.vertex_buffer_address = m_cube_mesh.vertex_buffer_address;

    std::vector<uint32_t> quad_indices{0, 2, 3, 0, 3, 1};
    std::vector<Vertex> quad_vertices{
        Vertex{glm::vec3{-0.5f, 0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, 0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 1.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{-0.5f, -0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, -0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}},
    };

    m_quad_mesh = core.allocate_mesh(quad_indices, quad_vertices);
    // m_push_constants.vertex_buffer_address = m_quad_mesh.vertex_buffer_address;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/mesh.vert.spv", "shaders/triangle.frag.spv", m_resources.layout.get(), vk::Format::eR16G16B16A16Sfloat,
        m_depth_img.format());
}

void SimpleCubeRenderer::render(const RenderFrames::FrameInfo &frame)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    m_depth_img.transition_layout(cmd, vk::ImageLayout::eDepthAttachmentOptimal);
    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(img.view())
                                .setImageLayout(img.layout())
                                .setLoadOp(vk::AttachmentLoadOp::eClear)
                                .setClearValue(clear)
                                .setStoreOp(vk::AttachmentStoreOp::eStore);
    auto depth_attachment =
        vk::RenderingAttachmentInfo{}
            .setImageView(m_depth_img.view())
            .setImageLayout(m_depth_img.layout())
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{}.setDepth(0.0f)));

    auto render_info = vk::RenderingInfo{}
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, img.extent()})
                           .setLayerCount(1)
                           .setColorAttachments(color_attachment)
                           .setPDepthAttachment(&depth_attachment);

    static float phi = 0.0f;
    phi += glm::quarter_pi<float>() * frame.delta_time / 1000.0f;

    m_push_constants.render_mtx =
        glm::mat4{{sin(phi), cos(phi), 0, 0}, {cos(phi), -sin(phi), 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

    cmd.beginRendering(render_info);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                     m_push_constants);

    auto viewport =
        vk::Viewport{}.setWidth(img.extent().width).setHeight(img.extent().height).setMinDepth(0.0f).setMaxDepth(1.0f);
    auto scissor = vk::Rect2D{}.setOffset(vk::Offset2D{0, 0}).setExtent(img.extent());

    cmd.setViewport(0, viewport);
    cmd.setScissor(0, scissor);

    cmd.bindIndexBuffer(m_cube_mesh.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(30, 1, 0, 0, 0);
    // cmd.bindIndexBuffer(m_quad_mesh.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    // cmd.drawIndexed(6, 1, 0, 0, 0);

    cmd.endRendering();
}
} // namespace ving
