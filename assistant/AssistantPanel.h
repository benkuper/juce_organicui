#pragma once
#include "AssistantController.h"

namespace organic::assistant
{
class Panel final : public ShapeShifterContentComponent, private juce::Timer, private juce::KeyListener, private juce::TextEditor::Listener
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
    class Pending;
    class CurrentActivity;
    class IconButton;
    class ActionButton;
    std::unique_ptr<Conversation> conversation;
    std::unique_ptr<History> history;
    std::unique_ptr<Pending> pending;
    std::unique_ptr<CurrentActivity> currentActivity;
    juce::Viewport viewport;
    juce::Viewport pendingViewport;
    std::unique_ptr<StringParameterTextUI> composer;
    std::unique_ptr<EnumParameterUI> model, access, effort;
    std::unique_ptr<IconButton> historyButton, settingsButton, newButton, deleteButton, copyButton, actionMenu;
    std::unique_ptr<ActionButton> primaryAction;
    std::unique_ptr<TriggerButtonUI> signIn;
    juce::TextButton reconnectButton {"Reconnect"};
    juce::TextButton steerButton {"Steer"}, resumeButton {"Resume unfinished"}, runQueueButton {"Run queue"};
    juce::OwnedArray<TriggerButtonUI> suggestions;
    String displayedTranscript, displayedConversation, statusText;
    juce::Rectangle<int> header, composerCard, emptyState;
    Activity activity = Activity::idle;
    bool isBusy = false, signedIn = false, showingHistory = false, turnActive = false;
    int conversationScroll = 0;
    double copiedUntil = 0;
    void showHistory(bool);
    void timerCallback() override;
    void updateActions();
    void showActionMenu();
    void textEditorTextChanged(juce::TextEditor&) override { updateActions(); }
    bool keyPressed(const juce::KeyPress&, juce::Component*) override;
};

class HubPanel final : public ShapeShifterContentComponent, private juce::Timer
{
public:
    HubPanel(const String&, Hub&);
    ~HubPanel() override;
    void resized() override;
private:
    Hub& hub;
    juce::WeakReference<ControllableContainer> owner;
    std::unique_ptr<EnumParameterUI> provider;
    juce::OwnedArray<Panel> panels;
    void timerCallback() override;
};
}
