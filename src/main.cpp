#include <iostream>

#include <vulkan/vulkan.hpp>

#include "ving_application.hpp"

const Uint8 *keys;
// HARD: Locking fov for now
constexpr float fov = 60.0f;

int main()
{
    try
    {
        ving::Application app{};
        app.run();
    }
    catch (vk::SystemError &e)
    {
        std::cerr << "Vulkan Exception thrown: " << e.what() << "\n";
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception thrown: " << e.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "Unkown error!\n";
    }
}
