#pragma once
#include "AssistantController.h"

namespace organic::assistant
{
class Panel final : public ShapeShifterContentComponent, private juce::Timer, private juce::KeyListener
{
public:
    Panel(const String& name, Controller&);
    ~Panel() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    Controller& controller;
    juce::WeakReference<ControllableContainer> owner;
    class Conversation;
    std::unique_ptr<Conversation> conversation;
    juce::Viewport viewport;
    std::unique_ptr<StringParameterTextUI> composer;
    std::unique_ptr<EnumParameterUI> model;
    std::unique_ptr<TriggerButtonUI> send, cancelButton, clear, settings, signIn;
    juce::OwnedArray<TriggerButtonUI> suggestions;
    String displayedTranscript, accountLabel, statusText;
    juce::Rectangle<int> header, footer, composerCard, emptyState;
    bool isBusy = false, hasAccount = false;
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress&, juce::Component*) override;
};
}
