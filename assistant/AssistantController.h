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
    ControllableContainer settings {"ChatGPT Assistant"};
    StringParameter *prompt, *transcript, *status;
    BoolParameter* busy;
    Trigger *send, *cancelRequest, *clearHistory, *newConversation, *openSettings;
    Trigger *explainProject, *findIssues, *buildWorkflow;
    EnumParameter *accounts, *models, *access, *effort;
    BoolParameter *advanced, *includeProjectContext;
    StringParameter* folders;
    Trigger *signIn, *addAccount, *signOut, *refreshModels, *saveAccess, *usage;
    void refresh();
    String currentError() const { return localError; }
    void selectConversation(const String& id) { perform([&] { provider.selectConversation(id); }); }
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
}
