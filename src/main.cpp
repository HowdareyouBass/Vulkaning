#include "ving_engine.hpp"

#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <iostream>

int main()
{
    try
    {
        ving::Engine engine{};

        engine.run();
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
