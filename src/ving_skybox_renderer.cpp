#include "ving_skybox_renderer.hpp"
#include "ving_color.hpp"

#include <ktx.h>

namespace ving
{
SkyboxRenderer::SkyboxRenderer(const Core &core, const Scene &scene) : r_core{core}
{
    auto resource_infos = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{ResourceIds::Skybox,
                                 {
                                     {0, vk::DescriptorType::eCombinedImageSampler},
                                     {1, vk::DescriptorType::eUniformBuffer},
                                 }},
    };

    m_resources = RenderResources{core.device(), resource_infos,
                                  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment};

    // m_skybox_cubemap = load_cube_map("assets/textures/cubemap_yokohama_rgba.ktx");
    m_skybox_cubemap = load_cube_map("assets/textures/skies.ktx");
    m_skybox_sampler = core.create_sampler(m_skybox_cubemap.mip_levels());

    m_resources.get_resource(ResourceIds::Skybox)
        .write_image(core.device(), 0, m_skybox_cubemap, m_skybox_sampler.get());

    m_quad = SimpleMesh::quad(core, colors::red);
    m_push_constants.vertex_buffer_address = m_quad.gpu_buffers.vertex_buffer_address;

    m_camera_info_buffer =
        core.create_cpu_visible_gpu_buffer(sizeof(CameraInfo), vk::BufferUsageFlagBits::eUniformBuffer);
    m_camera_info_buffer.map_data();
    m_camera_info = static_cast<CameraInfo *>(m_camera_info_buffer.data());

    m_resources.get_resource(ResourceIds::Skybox).write_buffer(core.device(), 1, m_camera_info_buffer);

    m_push_constants.light_direction = scene.light_direction;

    m_pipelines = core.create_graphics_render_pipelines<PushConstants>(
        "shaders/skybox.vert.spv", "shaders/skybox.frag.spv", m_resources.layouts(), vk::Format::eR16G16B16A16Sfloat,
        vk::Format::eUndefined, vk::PolygonMode::eFill);
}
void SkyboxRenderer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera)
{
    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    m_camera_info->forward = glm::normalize(camera.forward());
    m_camera_info->right = glm::normalize(camera.right());
    m_camera_info->up = glm::normalize(camera.up());
    m_camera_info->position = camera.position;

    img.transition_layout(cmd, vk::ImageLayout::eColorAttachmentOptimal);

    auto clear = vk::ClearValue{}.setColor(vk::ClearColorValue{0.1f, 0.1f, 0.1f, 1.0f});

    auto color_attachment = vk::RenderingAttachmentInfo{}
                                .setImageView(img.view())
                                .setImageLayout(img.layout())
                                .setLoadOp(vk::AttachmentLoadOp::eClear)
                                .setClearValue(clear)
                                .setStoreOp(vk::AttachmentStoreOp::eStore);

    auto render_info = vk::RenderingInfo{}
                           .setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, img.extent()})
                           .setLayerCount(1)
                           .setColorAttachments(color_attachment);

    cmd.beginRendering(render_info);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelines.layout.get(), 0, m_resources.descriptors(),
                           nullptr);
    cmd.pushConstants<PushConstants>(m_pipelines.layout.get(),
                                     vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                     m_push_constants);

    auto viewport =
        vk::Viewport{}.setWidth(img.extent().width).setHeight(img.extent().height).setMinDepth(0.0f).setMaxDepth(1.0f);
    auto scissor = vk::Rect2D{}.setOffset(vk::Offset2D{0, 0}).setExtent(img.extent());

    cmd.setViewport(0, viewport);
    cmd.setScissor(0, scissor);

    cmd.bindIndexBuffer(m_quad.gpu_buffers.index_buffer.buffer(), 0, vk::IndexType::eUint32);
    cmd.drawIndexed(m_quad.indices_count, 1, 0, 0, 0);

    cmd.endRendering();
}
Image2D SkyboxRenderer::load_cube_map(std::string_view filepath)
{
    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    ktxResult result;
    ktxTexture *ktx_texture;

    result = ktxTexture_CreateFromNamedFile(filepath.data(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
    if (result != KTX_SUCCESS)
        throw std::runtime_error("Failed to load skybox cubemap");

    ktx_uint8_t *ktx_texture_data = ktxTexture_GetData(ktx_texture);
    ktx_size_t ktx_texture_size = ktxTexture_GetDataSize(ktx_texture);

    GPUBuffer staging = r_core.create_cpu_visible_gpu_buffer(ktx_texture_size, vk::BufferUsageFlagBits::eTransferSrc);
    staging.map_data();
    void *data = staging.data();
    memcpy(data, ktx_texture_data, ktx_texture_size);
    staging.unmap_data();

    Image2D cube_map = r_core.create_image2d({ktx_texture->baseWidth, ktx_texture->baseHeight, 1}, format,
                                             vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                             vk::ImageLayout::eUndefined, ktx_texture->numLevels, 6,
                                             vk::ImageCreateFlagBits::eCubeCompatible);

    std::vector<vk::BufferImageCopy> buffer_copy_regions;
    uint32_t offset = 0;

    for (uint32_t face = 0; face < 6; ++face)
    {
        for (uint32_t level = 0; level < ktx_texture->numLevels; ++level)
        {
            ktx_size_t offset;
            KTX_error_code ret = ktxTexture_GetImageOffset(ktx_texture, level, 0, face, &offset);
            assert(ret == KTX_SUCCESS);

            auto image_subresource = vk::ImageSubresourceLayers{}
                                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                         .setMipLevel(level)
                                         .setBaseArrayLayer(face)
                                         .setLayerCount(1);

            auto copy = vk::BufferImageCopy{}
                            .setImageSubresource(image_subresource)
                            .setImageExtent({ktx_texture->baseWidth >> level, ktx_texture->baseHeight >> level, 1})
                            .setBufferOffset(offset);
            buffer_copy_regions.push_back(copy);
        }
    }

    r_core.immediate_transfer([&cube_map, &staging, &buffer_copy_regions](vk::CommandBuffer cmd) {
        cube_map.transition_layout(cmd, vk::ImageLayout::eTransferDstOptimal);
        cmd.copyBufferToImage(staging.buffer(), cube_map.image(), cube_map.layout(), buffer_copy_regions);
        cube_map.transition_layout(cmd, vk::ImageLayout::eShaderReadOnlyOptimal);
    });

    ktxTexture_Destroy(ktx_texture);

    return cube_map;
}
} // namespace ving
