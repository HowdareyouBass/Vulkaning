#pragma once

#include <SDL3/SDL_stdinc.h>

namespace ving
{
class Application
{
  private:
    static constexpr float fov = 60.0f;

    const Uint8 *keys;

  public:
    void run();
};
} // namespace ving
