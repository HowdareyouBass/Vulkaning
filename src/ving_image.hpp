#pragma once

namespace ving
{
class Image
{
  public:
    Image();

  private:
    vk::UniqueImage image;
    vk::UniqueImageView view;
    vk::ImageLayout layout;
};
} // namespace ving
