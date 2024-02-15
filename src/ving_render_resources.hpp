#pragma once

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "ving_gpu_buffer.hpp"
#include "ving_image.hpp"

namespace ving
{

// NOTE: Maybe should use string for key
struct RenderResourceCreateInfo
{
    uint32_t id;
    // NOTE: Done as pairs so it converts to unordered map
    std::vector<std::pair<uint32_t, vk::DescriptorType>> bindings;
};

class RenderResource
{
  public:
    RenderResource() = default;
    RenderResource(vk::DescriptorSet descriptor, std::unordered_map<uint32_t, vk::DescriptorType> bindings);

    void write_buffer(vk::Device device, uint32_t binding, const ving::GPUBuffer &buffer) const;
    void write_image(vk::Device device, uint32_t binding, const ving::Image2D &image, vk::Sampler sampler) const;
    void write_image(vk::Device device, uint32_t binding, vk::ImageView image, vk::ImageLayout layout,
                     vk::Sampler sampler) const;

  private:
    vk::DescriptorSet m_descriptor;
    std::unordered_map<uint32_t, vk::DescriptorType> m_bindings;
};

class RenderResources
{
  public:
    // WARN: Don't know what i did there but should work
    RenderResources() = default;
    RenderResources(vk::Device device, std::span<RenderResourceCreateInfo> infos, vk::ShaderStageFlags shader_stage);
    ~RenderResources();

    RenderResources(const RenderResources &) = delete;
    RenderResources &operator=(const RenderResources &) = delete;
    RenderResources(RenderResources &&) = delete;
    // RenderResources &operator=(RenderResources &&) = default;
    RenderResources &operator=(RenderResources &&rhs)
    {
        std::swap(m_pool, rhs.m_pool);
        m_layouts = std::move(rhs.m_layouts);
        m_descriptors = std::move(rhs.m_descriptors);
        m_resources = std::move(rhs.m_resources);
        std::swap(m_device, rhs.m_device);

        return *this;
    }

    void reset();

    [[nodiscard]] std::vector<vk::DescriptorSetLayout> layouts() const noexcept { return m_layouts; }
    [[nodiscard]] std::vector<vk::DescriptorSet> descriptors() const noexcept { return m_descriptors; }
    // NOTE: Assuming user will save his ids in enum
    [[nodiscard]] RenderResource get_resource(uint32_t id) const
    {
        assert(m_resources.find(id) != m_resources.end());
        return m_resources.find(id)->second;
    }

  private:
    vk::DescriptorPool m_pool;

    std::vector<vk::DescriptorSetLayout> m_layouts;
    // HACK: m_descriptors needed to bind to a pipeline
    // and m_resources to update or reset each descriptor indiviudally knowing id of it
    std::vector<vk::DescriptorSet> m_descriptors;
    std::unordered_map<uint32_t, RenderResource> m_resources;

    vk::Device m_device;
};

} // namespace ving
