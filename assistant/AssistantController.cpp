#include "AssistantController.h"
namespace organic::assistant
{
Controller::Controller(Provider& p) : ControllableContainer("Assistant"), provider(p), settings(p.info().name)
{
    auto metadata = provider.info(); settings.setCustomShortName(metadata.id);
    includeInRecursiveSave = false; nameCanBeChangedByUser = false;
    prompt = addStringParameter("Prompt", "Ctrl+Enter sends or queues a follow-up. Ctrl+Shift+Enter steers the active request.", ""); prompt->multiline = true;
    transcript = addStringParameter("Conversation", "Local conversation and tool activity.", ""); transcript->multiline = true;
    status = addStringParameter("Status", "Current assistant status.", "");
    busy = addBoolParameter("Busy", "An assistant request is running.", false);
    for (auto* value : std::initializer_list<Controllable*>{transcript, status, busy}) value->setControllableFeedbackOnly(true);
    for (auto* value : controllables) value->isSavable = false;
    send = addTrigger("Send", "Send the prompt, or queue it after the active request. Follow-ups pause when work needs attention.");
    steer = addTrigger("Steer", "Apply the prompt to the active request before its next project action (Ctrl+Shift+Enter).");
    resumeUnfinished = addTrigger("Resume unfinished", "Inspect saved changes and complete the remaining work in this conversation.");
    runQueue = addTrigger("Run queue", "Start the paused follow-ups in order, without resuming the unfinished request.");
    cancelRequest = addTrigger("Cancel", "Stop the active request and pause follow-ups. Completed project changes are kept.");
    clearHistory = addTrigger("Delete conversation", "Delete only the current local conversation.");
    newConversation = addTrigger("New conversation", "Start a new conversation and keep previous conversations in History.");
    openSettings = addTrigger("Account & access", "Open this provider under AI Assistant in Global Settings.");
    explainProject = addTrigger("Explain project", "Prepare a prompt to explain the current project.");
    findIssues = addTrigger("Find issues", "Prepare a read-only diagnostics prompt.");
    buildWorkflow = addTrigger("Build workflow", "Prepare a project authoring prompt.");
    accounts = settings.addEnumParameter("Account", "Account connected to this provider. Credentials remain in protected provider storage.");
    models = settings.addEnumParameter("Model", "Models offered by this provider.");
    effort = settings.addEnumParameter("Effort", "How much the selected model should think. Auto uses the model's default.");
    effort->addOption("Auto", "");
    signIn = settings.addTrigger(metadata.signInText, metadata.signInHint);
    addAccount = settings.addTrigger("Add account", "Register another ChatGPT account or workspace.");
    signOut = settings.addTrigger("Sign out", "Revoke the renewable session and remove local tokens; keep its account registration.");
    refreshModels = settings.addTrigger("Refresh models", "Refresh this provider's connection and model choices.");
    includeProjectContext = settings.addBoolParameter("Include project context", "Send a project summary to this provider with your conversation. Tools can request information within the access grant.", true);
    access = settings.addEnumParameter("Access", "Remembered permissions for this verified account's local tool calls.");
    access->addOption("Inspect", 0)->addOption("Control", 1)->addOption("Edit", 2)->addOption("Full Access", 3);
    advanced = settings.addBoolParameter("Advanced execution", "Allow direct script and system command execution. This is not a filesystem sandbox.", false);
    folders = settings.addStringParameter("Approved folders", "One absolute path per line for restricted profiles.", ""); folders->multiline = true;
    saveAccess = settings.addTrigger("Remember access", "Save this account's access profile for subsequent local tool calls.");
    usage = settings.addTrigger("Usage", "Open this provider's usage settings.");
    addAccount->hideInEditor = !metadata.multipleAccounts;
    if (!metadata.multipleAccounts) { signOut->setNiceName("Disconnect"); signOut->description = "Disconnect this provider from the assistant; keep the external application's own sign-in."; }
    settings.addControllableContainerListener(this);
    refresh(); startTimerHz(5);
}
Controller::~Controller() { stopTimer(); settings.removeControllableContainerListener(this); }
void Controller::perform(const std::function<void()>& task)
{
    try { localError.clear(); task(); refresh(); }
    catch (const mcp::Error& error) { localError = actionableError(String(error.what()), error.code); status->setValue(localError); }
    catch (const std::exception& error) { localError = actionableError(String(error.what())); status->setValue(localError); }
}
void Controller::onContainerTriggerTriggered(Trigger* trigger) { action(trigger); }
void Controller::onControllableFeedbackUpdate(ControllableContainer*, Controllable* value)
{
    if (updating) return;
    if (auto* trigger = dynamic_cast<Trigger*>(value)) action(trigger);
    else if (value == accounts) perform([this] { provider.select(accounts->getValueData().toString()); });
    else if (value == access)
    {
        const bool full = (int) access->getValueData() == 3;
        advanced->setEnabled(!full); folders->setEnabled(!full);
        if (full) advanced->setValue(true);
        perform([this]
        {
            auto paths = juce::StringArray::fromLines(folders->stringValue()); paths.trim(); paths.removeEmptyStrings();
            provider.setAccess((mcp::Access) (int) access->getValueData(), advanced->boolValue(), paths);
        });
    }
}
void Controller::action(Trigger* trigger)
{
    if (updating) return;
    perform([&]
    {
        if (trigger == send) { provider.send(prompt->stringValue(), models->getValueData().toString(), effort->getValueData().toString()); prompt->setValue(""); }
        else if (trigger == steer) { provider.steer(prompt->stringValue()); prompt->setValue(""); }
        else if (trigger == resumeUnfinished) { provider.resume(models->getValueData().toString(), effort->getValueData().toString(), prompt->stringValue()); prompt->setValue(""); }
        else if (trigger == runQueue) provider.runQueue();
        else if (trigger == cancelRequest) provider.cancel();
        else if (trigger == clearHistory) provider.clearHistory();
        else if (trigger == newConversation) { provider.newConversation(); prompt->setValue(""); }
        else if (trigger == openSettings) settings.selectThis();
        else if (trigger == signIn) provider.signIn(false);
        else if (trigger == addAccount) provider.signIn(true);
        else if (trigger == signOut) provider.signOut();
        else if (trigger == refreshModels) provider.refreshModels();
        else if (trigger == saveAccess)
        { auto paths = juce::StringArray::fromLines(folders->stringValue()); paths.trim(); paths.removeEmptyStrings(); provider.setAccess((mcp::Access) (int) access->getValueData(), advanced->boolValue(), paths); }
        else if (trigger == usage) juce::URL(provider.info().usageUrl).launchInDefaultBrowser();
        else if (trigger == explainProject) prompt->setValue("Explain this project's signal flow using observed objects and connections. Keep it read-only.");
        else if (trigger == findIssues) prompt->setValue("Inspect diagnostics for connection failures, missing assets, script errors and unresolved references. Report the evidence. Keep it read-only.");
        else if (trigger == buildWorkflow) prompt->setValue("Help me build a workflow. First inspect the existing project and available types, then describe the changes needed.");
    });
}
void Controller::editQueued(const String& id)
{
    perform([&]
    {
        if (!prompt->stringValue().trim().isEmpty()) throw mcp::Error(-32602, "Send or clear your current draft before editing a queued message.");
        auto view = provider.view();
        for (auto message : *view.pendingMessages.getArray()) if (message["id"].toString() == id)
        { provider.removeQueued(id); prompt->setValue(message["text"]); break; }
    });
}
void Controller::refresh()
{
    auto view = provider.view(); updating = true;
    auto statusText = view.status;
    if (!view.busy && view.activity == Activity::idle)
    {
        if (view.canResume) statusText = "Unfinished request saved - choose Resume unfinished";
        else if (view.queuePaused && view.pendingMessages.size() > 0) statusText = "Queue paused - choose Run queue";
    }
    busy->setValue(view.busy); status->setValue(localError.isEmpty() ? statusText : localError); transcript->setValue(view.transcript);
    auto signature = juce::JSON::toString(view.accounts, true);
    if (signature != accountSignature)
    {
        accountSignature = signature; juce::Array<EnumParameter::EnumValue> options;
        if (auto* list = view.accounts.getArray()) for (auto item : *list) options.add({item["label"].toString(), item["key"]});
        accounts->setOptions(options);
    }
    accounts->setValueWithData(view.active);
    signature = juce::JSON::toString(view.models, true);
    if (signature != modelSignature)
    {
        modelSignature = signature; auto previous = models->getValueData(); juce::Array<EnumParameter::EnumValue> options;
        if (auto* list = view.models.getArray()) for (auto item : *list) options.add({item["name"].toString(), item["slug"]});
        models->setOptions(options); if (!models->setValueWithData(previous) && !options.isEmpty()) models->setValueAtIndex(0);
    }
    if (selectedAccount != view.active)
    {
        selectedAccount = view.active;
        try { auto grant = provider.grant(); access->setValueWithData((int) grant.access); advanced->setValue(grant.advancedExecution || grant.access == mcp::Access::full); folders->setValue(grant.folders.joinIntoString("\n")); }
        catch (...) {}
    }
    auto selectedModel = models->getValueData().toString();
    if (effortModel != selectedModel)
    {
        effortModel = selectedModel; auto previous = effort->getValueData();
        juce::Array<EnumParameter::EnumValue> options; options.add({"Auto", ""});
        for (auto value : provider.efforts(selectedModel))
            options.add({value == "xhigh" ? String("Extra high") : value.substring(0, 1).toUpperCase() + value.substring(1), value});
        effort->setOptions(options); if (!effort->setValueWithData(previous)) effort->setValueWithData("");
    }
    send->setEnabled((!view.busy || view.turnActive) && models->getEntryForKey(models->getValueKey()) != nullptr);
    steer->setEnabled(view.turnActive);
    resumeUnfinished->setEnabled(!view.busy && view.canResume && models->getEntryForKey(models->getValueKey()) != nullptr);
    runQueue->setEnabled(!view.busy && view.pendingMessages.size() > 0);
    cancelRequest->setEnabled(view.busy);
    for (auto* control : std::initializer_list<Controllable*>{signIn, addAccount, signOut, refreshModels, accounts, models, access, clearHistory}) control->setEnabled(!view.busy);
    newConversation->setEnabled(true);
    effort->setEnabled(!view.busy && !provider.efforts(selectedModel).isEmpty());
    addAccount->setEnabled(!view.busy && provider.info().multipleAccounts);
    saveAccess->setEnabled(!view.active.isEmpty()); updating = false;
}
Hub::Hub(juce::Array<Controller*> entries) : ControllableContainer("AI Assistant"), controllers(entries)
{
    if (controllers.isEmpty()) throw mcp::Error(-32602, "The assistant needs at least one provider");
    includeInRecursiveSave = false; nameCanBeChangedByUser = false;
    providers = settings.addEnumParameter("Provider", "Choose the assistant provider. Each provider keeps its own settings and conversation history.");
    for (auto* controller : controllers)
    {
        auto metadata = controller->provider.info(); controller->setNiceName(metadata.name);
        providers->addOption(metadata.name, metadata.id);
        addChildControllableContainer(controller);
        settings.addChildControllableContainer(&controller->settings);
    }
    selected = controllers[0]; providers->setValueWithData(selected->provider.info().id);
    settings.addControllableContainerListener(this);
}
Hub::~Hub()
{
    settings.removeControllableContainerListener(this);
    for (auto* controller : controllers) { controller->provider.cancel(); settings.removeChildControllableContainer(&controller->settings); removeChildControllableContainer(controller); }
}
void Hub::onControllableFeedbackUpdate(ControllableContainer*, Controllable* control)
{
    if (control != providers) return;
    for (auto* candidate : controllers) if (candidate->provider.info().id == providers->getValueData().toString() && candidate != selected)
    {
        if (selected) selected->provider.cancel();
        selected = candidate; selected->refresh(); break;
    }
}
}
