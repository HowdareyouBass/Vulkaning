#include "ving_vulkan_raytracer.hpp"

namespace ving
{
VulkanRaytracer::VulkanRaytracer(const Core &core)
{
    auto acceleration_structure_info = vk::AccelerationStructureCreateInfoKHR{};

    m_mesh_acceleration_structure = core.device().createAccelerationStructureKHRUnique(acceleration_structure_info);

    auto device_address_info =
        vk::AccelerationStructureDeviceAddressInfoKHR{}.setAccelerationStructure(m_mesh_acceleration_structure.get());
    auto mesh_acceleration_structure_address = core.device().getAccelerationStructureAddressKHR(&device_address_info);

    auto transform = vk::TransformMatrixKHR{std::array<std::array<float, 4>, 3>{
        std::array<float, 4>{1.0f, 0.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 1.0f, 0.0f, 0.0f},
        std::array<float, 4>{0.0f, 0.0f, 1.0f, 0.0f},
    }};

    auto mesh_instance = vk::AccelerationStructureInstanceKHR{}
                             .setTransform(transform)
                             .setMask(0xFF)
                             .setInstanceShaderBindingTableRecordOffset(0)
                             .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
                             .setAccelerationStructureReference(mesh_acceleration_structure_address);
}
void VulkanRaytracer::render(const RenderFrames::FrameInfo &frame)
{
}
} // namespace ving
