#include "ving_vulkan_raytracer.hpp"

namespace ving
{
VulkanRaytracer::VulkanRaytracer(const Core &core)
{
    m_dispatch = core.create_dynamic_dispatch();

    auto transform = vk::TransformMatrixKHR{std::array<std::array<float, 4>, 3>{
        std::array<float, 4>{1.0f, 0.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 1.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 0.0f, 1.0f, 0.0f},
    }};

    m_cube = SimpleMesh::cube_raytraced(core, transform);

    auto acceleration_structure_geometry_data = vk::AccelerationStructureGeometryDataKHR{}.setTriangles(
        vk::AccelerationStructureGeometryTrianglesDataKHR{}
            .setVertexFormat(vk::Format::eR32G32B32Sfloat)
            .setVertexData(m_cube.vertex_buffer.device_address())
            .setMaxVertex(m_cube.vertices_count - 1)
            .setVertexStride(sizeof(Vertex))
            .setIndexType(vk::IndexType::eUint32)
            .setIndexData(m_cube.index_buffer.device_address())
            .setTransformData(vk::DeviceOrHostAddressConstKHR{}.setHostAddress(nullptr).setDeviceAddress(
                m_cube.transform_buffer.device_address())));

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
}

void VulkanRaytracer::render(const RenderFrames::FrameInfo &frame)
{
}
} // namespace ving
