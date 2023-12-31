#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
    , sdlHostRectangle_(juce::Rectangle<int>())
{
    juce::ignoreUnused (processorRef);

    sdlRunner_ = std::make_unique<SDLRunner>(*this);
    sdlRunner_->startThread();

    setSize(400, 300);
    setResizable(true, false);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
#if JUCE_WINDOWS
    if (juceHwndView.get() != nullptr)
    {
        juceHwndView->setHWND(nullptr);
    }
#elif JUCE_MAC
    if (juceNsView.get() != nullptr)
    {
        juceNsView->setView(nullptr);
    }
#elif JUCE_LINUX
    if (juceXEmbedView.get() != nullptr)
    {
        juceXEmbedView->removeClient();
    }
#endif

    sdlRunner_->stopThread(1000);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
#if 0
    auto scale_factor = 1.0;
    if (this->getPeer() != nullptr)
    {
        scale_factor = this->getPeer()->getPlatformScaleFactor();
    }
    sdlHostRectangle_ = getLocalBounds() * scale_factor;
#else
    sdlHostRectangle_ = getLocalBounds();
#endif

#if JUCE_WINDOWS
    if (juceHwndView.get() != nullptr)
    {
        juceHwndView->setBounds(sdlHostRectangle_);
    }
#elif JUCE_MAC
    if (juceNsView.get() != nullptr)
    {
        juceNsView->setBounds(sdlHostRectangle_);
    }
#elif JUCE_LINUX
    if (juceXEmbedView.get() != nullptr)
    {
        juceXEmbedView->setBounds(sdlHostRectangle_);
    }
#endif

    repaint();
}

juce::Rectangle<int> AudioPluginAudioProcessorEditor::getHostComponentRectangle() const
{
#if 0
    return sdlHostRectangle_;
#else
    auto scale_factor = 1.0;
    if (this->getPeer() != nullptr)
    {
        scale_factor = this->getPeer()->getPlatformScaleFactor();
    }
    return sdlHostRectangle_ * scale_factor;
#endif
    
}

void AudioPluginAudioProcessorEditor::onRendererCreated()
{
    juce::MessageManager::callAsync(
        [safe_this = juce::Component::SafePointer(this)] {
            if (safe_this.getComponent() == nullptr)
            {
                return;
            }

#if JUCE_WINDOWS
            safe_this->juceHwndView = std::make_unique<juce::HWNDComponent>();
            safe_this->juceHwndView->setHWND(safe_this->sdlRunner_->getNativeHandle());
            safe_this->addAndMakeVisible(safe_this->juceHwndView.get());
#elif JUCE_MAC
            safe_this->juceNsView = std::make_unique<juce::NSViewComponent>();
            safe_this->juceNsView->setView(safe_this->sdlRunner_->getNativeHandle());
            safe_this->addAndMakeVisible(safe_this->juceNsView.get());
#elif JUCE_LINUX
            safe_this->juceXEmbedView = std::make_unique<juce::XEmbedComponent>(safe_this->sdlRunner_->getNativeHandle());
            safe_this->addAndMakeVisible(juceXEmbedView.get());
#endif
            safe_this->resized();
        });
}
