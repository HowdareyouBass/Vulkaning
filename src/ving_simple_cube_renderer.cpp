#include "ving_simple_cube_renderer.hpp"
#include "ving_camera.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

    // clang-format off
    std::vector<uint32_t> cube_indices{0, 4, 5, 5, 1, 0,
                                       2, 6, 7, 7, 3, 2,
                                       1, 5, 6, 1, 6, 2,
                                       3, 7, 4, 0, 3, 4,
                                       4, 7, 5, 5, 7, 6,
                                       1, 2, 3, 0, 1, 3};
    // clang-format on

    std::vector<Vertex> cube_vertices{
        Vertex{glm::vec3{-0.5f, -0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, -0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, -0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}},
        Vertex{glm::vec3{-0.5f, -0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}},
        Vertex{glm::vec3{-0.5f, 0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, 0.5f, -0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{0.0f, 1.0f, 1.0f, 1.0f}},
        Vertex{glm::vec3{0.5f, 0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 1.0f, 0.0f, 1.0f}},
        Vertex{glm::vec3{-0.5f, 0.5f, 0.5f}, 0.0f, glm::vec3{}, 0.0f, glm::vec4{1.0f, 0.0f, 1.0f, 1.0f}},
    };
    GPUMeshBuffers cube_mesh_buffers = core.allocate_gpu_mesh_buffers(cube_indices, cube_vertices);
    Mesh cube_mesh = Mesh{std::move(cube_mesh_buffers), static_cast<uint32_t>(cube_indices.size())};
    m_cube = {std::move(cube_mesh), {}};
    m_push_constants.vertex_buffer_address = m_cube.mesh.gpu_buffers.vertex_buffer_address;

    std::string_view model_filepath = "assets/models/flat_vase.obj";
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err, warn;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, model_filepath.data());
    if (!warn.empty())
        std::cout << warn << std::endl;
    if (!err.empty())
        std::cout << err << std::endl;
    if (!ret)
        throw std::runtime_error("Failed to load model");

    std::vector<Vertex> model_vertices{};

    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
    {
        model_vertices.push_back(Vertex{{attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]},
                                        {},
                                        {},
                                        {},
                                        {1.0f, 1.0f, 1.0f, 1.0f}});
    }
    std::vector<uint32_t> model_indices{};

    for (size_t s = 0; s < shapes.size(); ++s)
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            for (size_t v = 0; v < fv; v++)
            {
                // Vertex new_vertex{};
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                model_indices.push_back(size_t(idx.vertex_index));
                // new_vertex.position = {attrib.vertices[idx.vertex_index], attrib.vertices[idx.vertex_index + 1],
                //                        attrib.vertices[idx.vertex_index + 2]};
                // new_vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};
                // model_vertices.push_back(new_vertex);

                if (idx.normal_index >= 0)
                {
                    model_vertices[idx.vertex_index].normal = {attrib.normals[idx.normal_index],
                                                               attrib.normals[idx.normal_index + 1],
                                                               attrib.normals[idx.normal_index + 2]};
                }
                if (idx.texcoord_index >= 0)
                {
                    model_vertices[idx.vertex_index].uv_x = attrib.texcoords[idx.texcoord_index];
                    model_vertices[idx.vertex_index].uv_y = attrib.texcoords[idx.texcoord_index + 1];
                }
            }
            index_offset += fv;
        }
    }

    GPUMeshBuffers model_mesh_buffers = core.allocate_gpu_mesh_buffers(model_indices, model_vertices);
    Mesh model_mesh = Mesh{std::move(model_mesh_buffers), static_cast<uint32_t>(model_indices.size())};
    m_model = {std::move(model_mesh), {}};
    m_push_constants.vertex_buffer_address = m_model.mesh.gpu_buffers.vertex_buffer_address;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/mesh.vert.spv", "shaders/triangle.frag.spv", m_resources.layout.get(), vk::Format::eR16G16B16A16Sfloat,
        m_depth_img.format());

    m_cube.transform.translation = glm::vec3{0.0f, 0.0f, 7.0f};
    // m_model.transform.scale = glm::vec3{0.1f};
}

void SimpleCubeRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera)
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

    m_cube.transform.rotation.x = phi;
    m_cube.transform.rotation.y = phi / 2.0f;

    auto projection_view = camera.projection() * camera.view();
    // m_push_constants.render_mtx = projection_view * m_cube.transform.mat4();
    // m_model.transform.rotation.x = phi;
    // m_model.transform.rotation.y = phi / 2.0f;
    m_push_constants.render_mtx = projection_view * m_model.transform.mat4();

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

    // cmd.bindIndexBuffer(m_cube.mesh.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    // cmd.drawIndexed(m_cube.mesh.indices_count, 1, 0, 0, 0);

    cmd.bindIndexBuffer(m_model.mesh.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(m_model.mesh.indices_count, 1, 0, 0, 0);

    cmd.endRendering();
}
} // namespace ving
