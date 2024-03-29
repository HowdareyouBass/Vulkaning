#include "ving_vulkan_raytracer.hpp"

namespace ving
{
VulkanRaytracer::VulkanRaytracer(const Core &core, RenderFrames &render_frames) : r_core{core}
{
    m_dispatch = core.create_dynamic_dispatch();

    auto bottom_transform = vk::TransformMatrixKHR{std::array<std::array<float, 4>, 3>{
        std::array<float, 4>{1.0f, 0.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 1.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 0.0f, 1.0f, 0.0f},
    }};

    m_cube = SimpleMesh::cube_raytraced(core, bottom_transform);

    auto acceleration_structure_geometry_data = vk::AccelerationStructureGeometryDataKHR{}.setTriangles(
        vk::AccelerationStructureGeometryTrianglesDataKHR{}
            .setVertexFormat(vk::Format::eR32G32B32Sfloat)
            .setVertexData(m_cube.vertex_buffer.device_address())
            .setMaxVertex(m_cube.vertices_count - 1)
            .setVertexStride(sizeof(Vertex))
            .setIndexType(vk::IndexType::eUint32)
            .setIndexData(m_cube.index_buffer.device_address())
            .setTransformData(m_cube.transform_buffer.device_address()));

    // auto acceleration_structure_geometry_data =
    // vk::AccelerationStructureGeometryDataKHR{}.setTriangles(vk::AccelerationStructureGeometryTrianglesDataKHR{}.setVertexFormat(vk::Format::eR);

    auto acceleration_structure_geomtery = vk::AccelerationStructureGeometryKHR{}
                                               .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                                               .setGeometryType(vk::GeometryTypeKHR::eTriangles)
                                               .setGeometry(acceleration_structure_geometry_data);

    auto acceleration_structure_build_geometry_info =
        vk::AccelerationStructureBuildGeometryInfoKHR{}
            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
            .setGeometries(acceleration_structure_geomtery);

    // HARD: Get triangles in a mesh
    const uint32_t num_triangles = 12;

    auto acceleration_structure_build_sizes_info = core.device().getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice, acceleration_structure_build_geometry_info, num_triangles,
        m_dispatch);

    m_bottom_accs_buffer = core.create_gpu_buffer(acceleration_structure_build_sizes_info.accelerationStructureSize,
                                                  vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                                                      vk::BufferUsageFlagBits::eShaderDeviceAddress);

    auto bottom_accs_info = vk::AccelerationStructureCreateInfoKHR{}
                                .setBuffer(m_bottom_accs_buffer.buffer())
                                .setSize(acceleration_structure_build_sizes_info.accelerationStructureSize)
                                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel);

    m_bottom_accs = core.device().createAccelerationStructureKHRUnique(bottom_accs_info, nullptr, m_dispatch);

    auto bottom_scratch_buffer =
        core.create_gpu_buffer(acceleration_structure_build_sizes_info.buildScratchSize,
                               vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);

    auto accs_build_geometry_info = vk::AccelerationStructureBuildGeometryInfoKHR{}
                                        .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                                        .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                                        .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                                        .setDstAccelerationStructure(m_bottom_accs.get())
                                        .setGeometries(acceleration_structure_geomtery)
                                        .setScratchData(bottom_scratch_buffer.device_address());

    auto accs_build_range = vk::AccelerationStructureBuildRangeInfoKHR{}.setPrimitiveCount(num_triangles);

    render_frames.immediate_submit([&accs_build_geometry_info, &accs_build_range, this](vk::CommandBuffer cmd) {
        cmd.buildAccelerationStructuresKHR(accs_build_geometry_info, &accs_build_range, m_dispatch);
    });

    auto accs_device_address_info =
        vk::AccelerationStructureDeviceAddressInfoKHR{}.setAccelerationStructure(m_bottom_accs.get());

    m_bottom_accs_address = core.device().getAccelerationStructureAddressKHR(accs_device_address_info, m_dispatch);

    // Top level acceleration structure
    auto top_transform = vk::TransformMatrixKHR{std::array<std::array<float, 4>, 3>{
        std::array<float, 4>{1.0f, 0.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 1.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 0.0f, 1.0f, 0.0f},
    }};

    auto instance = vk::AccelerationStructureInstanceKHR{}
                        .setTransform(top_transform)
                        .setMask(0xFF)
                        .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
                        .setAccelerationStructureReference(m_bottom_accs_address);

    m_instances_buffer =
        core.create_gpu_buffer(&instance, sizeof(vk::AccelerationStructureInstanceKHR),
                               vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                   vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

    auto top_accs_geometry_data = vk::AccelerationStructureGeometryDataKHR{}.setInstances(
        vk::AccelerationStructureGeometryInstancesDataKHR{}.setData(m_instances_buffer.device_address()));

    auto top_accs_geometry = vk::AccelerationStructureGeometryKHR{}
                                 .setGeometryType(vk::GeometryTypeKHR::eInstances)
                                 .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                                 .setGeometry(top_accs_geometry_data);

    auto top_accs_build_geometry_info = vk::AccelerationStructureBuildGeometryInfoKHR{}
                                            .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                                            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                                            .setGeometries(top_accs_geometry);

    // HARD: Should be vector?
    uint32_t instances_count = 1;

    auto top_accs_build_sizes = core.device().getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice, top_accs_build_geometry_info, instances_count, m_dispatch);

    m_top_accs_buffer = core.create_gpu_buffer(top_accs_build_sizes.accelerationStructureSize,
                                               vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);

    auto top_accs_info = vk::AccelerationStructureCreateInfoKHR{}
                             .setBuffer(m_top_accs_buffer.buffer())
                             .setSize(top_accs_build_sizes.accelerationStructureSize)
                             .setType(vk::AccelerationStructureTypeKHR::eTopLevel);

    m_top_accs = core.device().createAccelerationStructureKHRUnique(top_accs_info, nullptr, m_dispatch);

    auto top_scratch_buffer =
        core.create_gpu_buffer(top_accs_build_sizes.buildScratchSize,
                               vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress);

    auto top_accs_build_info = vk::AccelerationStructureBuildGeometryInfoKHR{}
                                   .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                                   .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                                   .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                                   .setDstAccelerationStructure(m_top_accs.get())
                                   .setGeometries(top_accs_geometry)
                                   .setScratchData(top_scratch_buffer.device_address());

    auto top_accs_build_range = vk::AccelerationStructureBuildRangeInfoKHR{}.setPrimitiveCount(num_triangles);

    render_frames.immediate_submit([&top_accs_build_info, &top_accs_build_range, this](vk::CommandBuffer cmd) {
        cmd.buildAccelerationStructuresKHR(top_accs_build_info, &top_accs_build_range, m_dispatch);
    });

    auto top_accs_device_address_info =
        vk::AccelerationStructureDeviceAddressInfoKHR{}.setAccelerationStructure(m_top_accs.get());

    m_top_accs_address = core.device().getAccelerationStructureAddressKHR(&top_accs_device_address_info, m_dispatch);

    auto resource_create_infos = std::vector<RenderResourceCreateInfo>{
        RenderResourceCreateInfo{RenderResourceIds::Main,
                                 {
                                     {0, vk::DescriptorType::eAccelerationStructureKHR},
                                     {1, vk::DescriptorType::eStorageImage},
                                     {2, vk::DescriptorType::eUniformBuffer},
                                 }},
    };

    m_resources = RenderResources{core.device(), resource_create_infos, vk::ShaderStageFlagBits::eRaygenKHR};

    m_resources.get_resource(RenderResourceIds::Main).write_acceleration_structure(core.device(), 0, m_top_accs.get());

    m_render_image = core.create_image2d(vk::Extent3D{core.get_window_extent(), 1}, vk::Format::eR16G16B16A16Sfloat,
                                         vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage);
    m_resources.get_resource(RenderResourceIds::Main)
        .write_image(core.device(), 1, m_render_image, vk::ImageLayout::eGeneral);

    m_ubo_buffer = core.create_cpu_visible_gpu_buffer(sizeof(Ubo), vk::BufferUsageFlagBits::eUniformBuffer);
    m_ubo_buffer.map_data();
    m_ubo = static_cast<Ubo *>(m_ubo_buffer.data());

    m_resources.get_resource(RenderResourceIds::Main).write_buffer(core.device(), 2, m_ubo_buffer);

    m_pipelines = core.create_ray_tracing_pipelines<PushConstants>(
        "shaders/bin/basic.rgen.spv", "shaders/bin/basic.rahit.spv", "shaders/bin/basic.rchit.spv",
        "shaders/bin/basic.rmiss.spv", "shaders/bin/basic.rint.spv", 1, m_resources.layouts(), m_dispatch);

    create_binding_table();
}

void VulkanRaytracer::create_binding_table()
{
    auto properties = vk::PhysicalDeviceProperties2{};
    properties.pNext = &m_raytracing_pipeline_properties;
    r_core.gpu().getProperties2(&properties);

    auto features = vk::PhysicalDeviceFeatures2{}.setPNext(&m_acceleration_structure_features);
    r_core.gpu().getFeatures2(&features);

    // TODO: Move it inside pipeline creation
    const uint32_t handle_size = m_raytracing_pipeline_properties.shaderGroupHandleSize;
    const uint32_t handle_size_aligned =
        utils::aligned_size(m_raytracing_pipeline_properties.shaderGroupHandleSize,
                            m_raytracing_pipeline_properties.shaderGroupHandleAlignment);
    const uint32_t handle_alignment = m_raytracing_pipeline_properties.shaderGroupHandleAlignment;
    const uint32_t group_count = m_pipelines.shader_groups_count;
    const uint32_t sbt_size = group_count * handle_size_aligned;
    const vk::BufferUsageFlags sbt_buffer_usage_flags = vk::BufferUsageFlagBits::eShaderBindingTableKHR |
                                                        vk::BufferUsageFlagBits::eTransferSrc |
                                                        vk::BufferUsageFlagBits::eShaderDeviceAddress;

    std::vector<uint8_t> shader_handle_storage(sbt_size);

    auto handles = r_core.device().getRayTracingShaderGroupHandlesKHR(
        m_pipelines.pipeline.get(), 0, group_count, sbt_size, shader_handle_storage.data(), m_dispatch);

    m_raygen_sbt_buffer = r_core.create_cpu_visible_gpu_buffer(handle_size, sbt_buffer_usage_flags);
    m_raygen_sbt_buffer.set_memory(r_core.device(), shader_handle_storage.data(), handle_size);

    m_miss_sbt_buffer = r_core.create_cpu_visible_gpu_buffer(handle_size, sbt_buffer_usage_flags);
    m_miss_sbt_buffer.set_memory(r_core.device(), shader_handle_storage.data() + handle_size_aligned, handle_size);

    m_hit_sbt_buffer = r_core.create_cpu_visible_gpu_buffer(handle_size, sbt_buffer_usage_flags);
    m_hit_sbt_buffer.set_memory(r_core.device(), shader_handle_storage.data() + handle_size_aligned * 2, handle_size);
}

void VulkanRaytracer::render(const RenderFrames::FrameInfo &frame, const PerspectiveCamera &camera)
{
    m_ubo->view_inverse = camera.view();
    m_ubo->proj_inverse = camera.projection();

    vk::CommandBuffer cmd = frame.cmd;
    Image2D &img = frame.draw_image;

    m_render_image.transition_layout(cmd, vk::ImageLayout::eGeneral);
    img.transition_layout(cmd, vk::ImageLayout::eGeneral);

    const uint32_t handle_size_aligned =
        utils::aligned_size(m_raytracing_pipeline_properties.shaderGroupHandleSize,
                            m_raytracing_pipeline_properties.shaderGroupHandleAlignment);

    auto raygen_shader_sbt_entry = vk::StridedDeviceAddressRegionKHR{}
                                       .setDeviceAddress(m_raygen_sbt_buffer.device_address())
                                       .setStride(handle_size_aligned)
                                       .setSize(handle_size_aligned);
    auto miss_shader_sbt_entry = vk::StridedDeviceAddressRegionKHR{}
                                     .setDeviceAddress(m_miss_sbt_buffer.device_address())
                                     .setStride(handle_size_aligned)
                                     .setSize(handle_size_aligned);
    auto hit_shader_sbt_entry = vk::StridedDeviceAddressRegionKHR{}
                                    .setDeviceAddress(m_hit_sbt_buffer.device_address())
                                    .setStride(handle_size_aligned)
                                    .setSize(handle_size_aligned);

    vk::StridedDeviceAddressRegionKHR callable_shader_sbt_entry{};

    cmd.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_pipelines.pipeline.get());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelines.layout.get(), 0,
                           m_resources.descriptors(), nullptr);
    cmd.traceRaysKHR(raygen_shader_sbt_entry, miss_shader_sbt_entry, hit_shader_sbt_entry, callable_shader_sbt_entry,
                     r_core.get_window_extent().width, r_core.get_window_extent().height, 1, m_dispatch);

    m_render_image.transition_layout(cmd, vk::ImageLayout::eTransferSrcOptimal);
    img.transition_layout(cmd, vk::ImageLayout::eTransferDstOptimal);

    m_render_image.copy_to(cmd, img);
}
} // namespace ving
