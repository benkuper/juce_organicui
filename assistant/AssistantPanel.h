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
    String conversationText() const;
private:
    Controller& controller;
    juce::WeakReference<ControllableContainer> owner;
    class Conversation;
    class History;
    class IconButton;
    std::unique_ptr<Conversation> conversation;
    std::unique_ptr<History> history;
    juce::Viewport viewport;
    std::unique_ptr<StringParameterTextUI> composer;
    std::unique_ptr<EnumParameterUI> model, access, effort;
    std::unique_ptr<IconButton> send, stop, historyButton, settingsButton, newButton, deleteButton, copyButton;
    std::unique_ptr<TriggerButtonUI> signIn;
    juce::TextButton usageButton {"Usage"};
    juce::TextButton reconnectButton {"Reconnect"};
    juce::Label statusLabel;
    juce::OwnedArray<TriggerButtonUI> suggestions;
    String displayedTranscript, displayedConversation, statusText;
    juce::Rectangle<int> header, statusLine, composerCard, emptyState;
    Activity activity = Activity::idle;
    bool isBusy = false, signedIn = false, showingHistory = false;
    int conversationScroll = 0;
    double copiedUntil = 0;
    void showHistory(bool);
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress&, juce::Component*) override;
};
}
