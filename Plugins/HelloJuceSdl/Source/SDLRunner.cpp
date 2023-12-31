#include "SDLRunner.h"

namespace sdl2
{
#include <SDL.h>
#include <SDL_version.h>
#include <SDL_syswm.h>
#include <SDL_video.h>
}

namespace
{
    void drawScene(sdl2::SDL_Renderer* renderer, int width, int height, int mouseX, int mouseY, bool mouseOn)
    {
        {
            sdl2::SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            sdl2::SDL_Rect rect{ width * 0.2f, height * 0.2f, width * 0.6f, height * 0.6f };
            sdl2::SDL_RenderFillRect(renderer , &rect);
        }

        {
            sdl2::SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            sdl2::SDL_Rect rect{ mouseX - 8, mouseY - 8, 16, 16 };
            sdl2::SDL_RenderFillRect(renderer, &rect);
        }

        if (mouseOn)
        {
            sdl2::SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            sdl2::SDL_Rect rect{ mouseX - 8, mouseY - 8, 16, 16 };
            sdl2::SDL_RenderFillRect(renderer, &rect);
        }
    }
}

SDLRunner::SDLRunner(IHostComponentInformationProvider& hostComponent)
    : juce::Thread("SDL Runner Thread")
    , hostComponentRef_(hostComponent)
{
}

SDLRunner::~SDLRunner()
{
}

void* SDLRunner::getNativeHandle() const
{
    std::unique_lock lock(mutex_);

    sdl2::SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    sdl2::SDL_GetWindowWMInfo(sdlWindow_.load(), &wmInfo);
    void* hwnd = wmInfo.info.win.window;

    return hwnd;
}

void SDLRunner::setBounds(juce::Rectangle<int> bounds)
{
    bounds_ = bounds;
}

void SDLRunner::run()
{
    if (sdl2::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        return;
    }

    // Scoped lock.
    {
        std::unique_lock lock(mutex_);

        // Create window.
        {
            typedef sdl2::Uint32 SDL_WindowFlags;
            SDL_WindowFlags flags = sdl2::SDL_WINDOW_SHOWN |
                sdl2::SDL_WINDOW_BORDERLESS |
                sdl2::SDL_WINDOW_ALLOW_HIGHDPI |
                sdl2::SDL_WINDOW_RESIZABLE;
            auto* window = sdl2::SDL_CreateWindow("SDL View", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, flags);
            if (window == nullptr)
            {
                this->signalThreadShouldExit();
            }
            else
            {
                sdlWindow_.exchange(window);
            }
        }

        // Create renderer.
        if (sdlWindow_.load() != nullptr)
        {
            typedef sdl2::Uint32 SDL_RendererFlags;
            SDL_RendererFlags flags = sdl2::SDL_RENDERER_SOFTWARE;
            //SDL_RendererFlags flags = sdl2::SDL_RENDERER_ACCELERATED;
            auto* renderer = sdl2::SDL_CreateRenderer(sdlWindow_.load(), -1, flags);
            if (renderer == nullptr)
            {
                this->signalThreadShouldExit();
            }
            else
            {
                sdlRenderer_.exchange(renderer);
            }
        }
    }

    hostComponentRef_.onRendererCreated();

    bool quit = this->threadShouldExit();

    // Create state store.
    sdl2::SDL_Event e;
    int mouse_x = 0;
    int mouse_y = 0;
    bool mouse_button_down = false;
    int width = 0;
    int height = 0;

    // SDL event loop.
    while (!quit)
    {
        entereRunLoop_ = true;

        while (sdl2::SDL_PollEvent(&e) != 0)
        {
            if (e.type == sdl2::SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == sdl2::SDL_WINDOWEVENT)
            {
                const auto rect_host = hostComponentRef_.getHostComponentRectangle();
                sdl2::SDL_SetWindowSize(sdlWindow_.load(), rect_host.getWidth(), rect_host.getHeight());
            }
            else if (e.type == sdl2::SDL_MOUSEMOTION)
            {
                sdl2::SDL_GetMouseState(&mouse_x, &mouse_y);
            }
            else if (e.type == sdl2::SDL_MOUSEBUTTONDOWN)
            {
                mouse_button_down = true;
            }
            else if (e.type == sdl2::SDL_MOUSEBUTTONUP)
            {
                mouse_button_down = false;
            }
        }

        // Rendering process.
        {
            sdl2::SDL_SetRenderDrawColor(sdlRenderer_.load(), 255, 255, 255, 255);
            sdl2::SDL_RenderClear(sdlRenderer_.load());

            sdl2::SDL_GetWindowSize(sdlWindow_.load(), &width, &height);

            drawScene(sdlRenderer_.load(), width, height, mouse_x, mouse_y, mouse_button_down);

            sdl2::SDL_RenderPresent(sdlRenderer_.load());
        }

        // Update thread should exit or not.
        quit = this->threadShouldExit();

        juce::Thread::sleep(1);
    }

    // Scoped lock.
    {
        std::unique_lock lock(mutex_);

        sdl2::SDL_DestroyRenderer(sdlRenderer_.exchange(nullptr));
        sdl2::SDL_DestroyWindow(sdlWindow_.exchange(nullptr));
    }

    sdl2::SDL_Quit();
}
