#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
namespace sdl2
{
    struct SDL_Window;
    struct SDL_Renderer;
}

//==============================================================================
class IHostComponentInformationProvider
{
public:
    //==============================================================================
    virtual juce::Rectangle<int> getHostComponentRectangle() const = 0;
    virtual void onRendererCreated() = 0;

private:
    //==============================================================================

    JUCE_LEAK_DETECTOR(IHostComponentInformationProvider)
};

//==============================================================================
class SDLRunner
    : public juce::Thread
{
public:
    //==============================================================================
    explicit SDLRunner(IHostComponentInformationProvider& hostComponent);
    ~SDLRunner() override;

    //==============================================================================
    void* getNativeHandle() const;

    void setBounds(juce::Rectangle<int> bounds);

private:
    //==============================================================================
    virtual void run() override;

    //==============================================================================
    mutable std::mutex mutex_;

    std::atomic<sdl2::SDL_Window*> sdlWindow_;
    std::atomic<sdl2::SDL_Renderer*> sdlRenderer_;
    std::atomic<bool> entereRunLoop_;

    IHostComponentInformationProvider& hostComponentRef_;
    juce::Rectangle<int> bounds_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SDLRunner)
};