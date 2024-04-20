#include <iostream>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>

#include "ving_application.hpp"

int main()
{
    try
    {
        if (SDL_Init(SDL_InitFlags::SDL_INIT_VIDEO) < 0)
            throw std::runtime_error(std::format("Couldn't initialize SDL: {}", SDL_GetError()));

        SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 1080, 1080, SDL_WINDOW_VULKAN);
        // SDL_Window *window = SDL_CreateWindow("No title in dwm :(", 1280, 720, SDL_WINDOW_VULKAN);
        if (!window)
            throw std::runtime_error(std::format("Failed to create SDL window: {}", SDL_GetError()));

        ving::Application app{window};
        app.run();
        while (app.running())
        {
            app.update();
        }
        // FIXME: Looks bad why then i did the class if it's not self destructing
        app.stop();
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
