#pragma once
#include "AssistantProvider.h"

namespace organic::assistant
{
// Conversation controls belong to the application model. Account, model and
// access preferences are a separate container attached to Global Settings.
// Token material is exclusively in the provider's protected settings backing store.
class Controller final : public ControllableContainer, private juce::Timer
{
public:
    explicit Controller(Provider&);
    ~Controller() override;
    Provider& provider;
    ControllableContainer settings;
    StringParameter *prompt, *transcript, *status;
    BoolParameter* busy;
    Trigger *send, *steer, *resumeUnfinished, *runQueue, *cancelRequest, *clearHistory, *newConversation, *openSettings;
    Trigger *explainProject, *findIssues, *buildWorkflow;
    EnumParameter *accounts, *models, *access, *effort;
    BoolParameter *advanced, *includeProjectContext;
    StringParameter* folders;
    Trigger *signIn, *addAccount, *signOut, *refreshModels, *saveAccess, *usage;
    void refresh();
    String currentError() const { return localError; }
    void selectConversation(const String& id) { perform([&] { provider.selectConversation(id); }); }
    void removeQueued(const String& id) { perform([&] { provider.removeQueued(id); }); }
    void steerQueued(const String& id) { perform([&] { provider.steerQueued(id); }); }
    void editQueued(const String& id);
    void onContainerTriggerTriggered(Trigger*) override;
    void onControllableFeedbackUpdate(ControllableContainer*, Controllable*) override;
private:
    bool updating = false;
    String selectedAccount, accountSignature, modelSignature, localError;
    String effortModel;
    void timerCallback() override { refresh(); }
    void action(Trigger*);
    void perform(const std::function<void()>&);
};

// One application/settings entry, with independent sessions for each provider.
class Hub final : public ControllableContainer
{
public:
    explicit Hub(juce::Array<Controller*>);
    ~Hub() override;
    ControllableContainer settings {"AI Assistant"};
    EnumParameter* providers;
    const juce::Array<Controller*> controllers;
    Controller& active() const { return *selected; }
    void onControllableFeedbackUpdate(ControllableContainer*, Controllable*) override;
private:
    Controller* selected = nullptr;
};
}
