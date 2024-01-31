#include "ving_defaults.hpp"

namespace ving
{
namespace def
{
vk::ImageSubresourceRange image_subresource_range_no_mip_no_levels(vk::ImageAspectFlags aspect_mask)
{
    return vk::ImageSubresourceRange{}
        .setAspectMask(aspect_mask)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);
}

} // namespace def
} // namespace ving
