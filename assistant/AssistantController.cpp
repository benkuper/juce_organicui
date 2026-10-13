#include "AssistantController.h"
namespace organic::assistant
{
Controller::Controller(Provider& p) : ControllableContainer("Assistant"), provider(p)
{
    includeInRecursiveSave = false; nameCanBeChangedByUser = false;
    prompt = addStringParameter("Prompt", "Message for the assistant. Ctrl+Enter sends from the conversation panel.", ""); prompt->multiline = true;
    transcript = addStringParameter("Conversation", "Local conversation and tool activity.", ""); transcript->multiline = true;
    status = addStringParameter("Status", "Current assistant status.", "");
    busy = addBoolParameter("Busy", "An assistant request is running.", false);
    for (auto* value : std::initializer_list<Controllable*>{transcript, status, busy}) value->setControllableFeedbackOnly(true);
    for (auto* value : controllables) value->isSavable = false;
    send = addTrigger("Send", "Send the prompt using the selected ChatGPT model and remembered access.");
    cancelRequest = addTrigger("Cancel", "Cancel pending inference and queued tool operations.");
    clearHistory = addTrigger("Clear conversation", "Delete this account's local conversation history.");
    openSettings = addTrigger("Account & access", "Open ChatGPT Assistant in Global Settings.");
    explainProject = addTrigger("Explain project", "Prepare a prompt to explain the current project.");
    findIssues = addTrigger("Find issues", "Prepare a read-only diagnostics prompt.");
    buildWorkflow = addTrigger("Build workflow", "Prepare a project authoring prompt.");
    accounts = settings.addEnumParameter("Account", "Saved, verified ChatGPT registrations. Credentials are protected in local Global Settings storage.");
    models = settings.addEnumParameter("Model", "Models available to the selected ChatGPT account.");
    signIn = settings.addTrigger("Continue with ChatGPT", "Sign in through your browser using your ChatGPT plan. Account integration is in preview.");
    addAccount = settings.addTrigger("Add account", "Register another ChatGPT account or workspace.");
    signOut = settings.addTrigger("Sign out", "Revoke the renewable session and remove local tokens; keep its account registration.");
    refreshModels = settings.addTrigger("Refresh models", "Reload account-specific ChatGPT model choices.");
    includeProjectContext = settings.addBoolParameter("Include project context", "Send a project summary to OpenAI with your conversation. Tool calls can request further information within the access grant.", true);
    access = settings.addEnumParameter("Access", "Remembered permissions for this verified account's local tool calls.");
    access->addOption("Inspect", 0)->addOption("Control", 1)->addOption("Edit", 2)->addOption("Full Access", 3);
    advanced = settings.addBoolParameter("Advanced execution", "Allow direct script and system command execution. This is not a filesystem sandbox.", false);
    folders = settings.addStringParameter("Approved folders", "One absolute path per line for restricted profiles.", ""); folders->multiline = true;
    saveAccess = settings.addTrigger("Remember access", "Save this account's access profile for subsequent local tool calls.");
    usage = settings.addTrigger("ChatGPT usage", "Open ChatGPT Settings to review plan eligibility and app usage limits.");
    settings.addControllableContainerListener(this);
    refresh(); startTimerHz(5);
}
Controller::~Controller() { stopTimer(); settings.removeControllableContainerListener(this); }
void Controller::perform(const std::function<void()>& task)
{
    try { localError.clear(); task(); refresh(); }
    catch (const std::exception& error) { localError = String(error.what()); status->setValue(localError); }
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
    }
}
void Controller::action(Trigger* trigger)
{
    if (updating) return;
    perform([&]
    {
        if (trigger == send) { provider.send(prompt->stringValue(), models->getValueData().toString()); prompt->setValue(""); }
        else if (trigger == cancelRequest) provider.cancel();
        else if (trigger == clearHistory) provider.clearHistory();
        else if (trigger == openSettings) settings.selectThis();
        else if (trigger == signIn) provider.signIn(false);
        else if (trigger == addAccount) provider.signIn(true);
        else if (trigger == signOut) provider.signOut();
        else if (trigger == refreshModels) provider.refreshModels();
        else if (trigger == saveAccess)
        { auto paths = juce::StringArray::fromLines(folders->stringValue()); paths.trim(); paths.removeEmptyStrings(); provider.setAccess((mcp::Access) (int) access->getValueData(), advanced->boolValue(), paths); }
        else if (trigger == usage) juce::URL("https://chatgpt.com/settings/usage").launchInDefaultBrowser();
        else if (trigger == explainProject) prompt->setValue("Explain this project's signal flow using observed objects and connections. Keep it read-only.");
        else if (trigger == findIssues) prompt->setValue("Inspect diagnostics for connection failures, missing assets, script errors and unresolved references. Report the evidence. Keep it read-only.");
        else if (trigger == buildWorkflow) prompt->setValue("Help me build a workflow. First inspect the existing project and available types, then describe the changes needed.");
    });
}
void Controller::refresh()
{
    auto view = provider.view(); updating = true;
    busy->setValue(view.busy); status->setValue(localError.isEmpty() ? view.status : localError); transcript->setValue(view.transcript);
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
    send->setEnabled(!view.busy && models->getEntryForKey(models->getValueKey()) != nullptr);
    cancelRequest->setEnabled(view.busy);
    for (auto* control : std::initializer_list<Controllable*>{signIn, addAccount, signOut, refreshModels, accounts, models}) control->setEnabled(!view.busy);
    saveAccess->setEnabled(!view.active.isEmpty()); updating = false;
}
}
