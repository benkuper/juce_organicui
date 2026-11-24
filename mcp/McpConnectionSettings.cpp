#include "McpConnectionSettings.h"
namespace organic::mcp
{
ConnectionSettings::ConnectionSettings(Permissions& p, Server& s, std::function<int()> port)
    : ControllableContainer("Clients"), permissions(p), server(s), port(port)
{
    clients = addEnumParameter("Client", "One remembered access grant per authenticated credential.");
    clientName = addStringParameter("Client name", "Name for a new client.", "");
    profile = addEnumParameter("Access", "Remembered access; authorized calls need no further application prompts.");
    profile->addOption("Inspect", 0)->addOption("Control", 1)->addOption("Edit", 2)->addOption("Full Access", 3);
    advanced = addBoolParameter("Advanced execution", "Permit direct scripts and system commands. This is not a filesystem sandbox.", false);
    folders = addStringParameter("Approved folders", "One absolute path per line for restricted profiles.", ""); folders->multiline = true;
    create = addTrigger("Create client", "Create a credential and remember this access profile.");
    save = addTrigger("Save access", "Update the selected client's remembered access.");
    rotate = addTrigger("Rotate credential", "Revoke the old credential and issue a replacement.");
    revoke = addTrigger("Revoke client", "Immediately revoke this client and its observation streams.");
    copyToken = addTrigger("Copy new credential", "Copy the newly created bearer to the clipboard once; store it securely in your client.");
    forgetToken = addTrigger("Forget credential", "Remove the newly issued credential from this settings session.");
    copyCodex = addTrigger("Copy Codex config", "Copy HTTP configuration using CHATAIGNE_MCP_TOKEN from the client's environment.");
    copyClaude = addTrigger("Copy Claude Code config", "Copy HTTP configuration using CHATAIGNE_MCP_TOKEN from the client's environment.");
    test = addTrigger("Test connection", "Test this instance using the newly created credential.");
    result = addStringParameter("Connection result", "Result of the last connection action.", "Create a client, then copy its credential.");
    activity = addStringParameter("Activity", "Recent request methods, status and duration. Credentials and payloads are excluded.", ""); activity->multiline = true;
    for (auto* value : {result, activity}) { value->setControllableFeedbackOnly(true); value->isSavable = false; }
    // Credentials/grants have a dedicated fingerprint store; don't serialize an
    // editor's incomplete access changes as another source of permissions.
    for (auto* value : controllables) value->isSavable = false;
    refreshClients(); startTimerHz(2);
}
ConnectionSettings::~ConnectionSettings() { stopTimer(); newToken.clear(); }
String ConnectionSettings::endpoint() const { return "http://127.0.0.1:" + String(port()) + "/mcp"; }
juce::StringArray ConnectionSettings::approvedFolders() const
{
    auto paths = juce::StringArray::fromLines(folders->stringValue()); paths.trim(); paths.removeEmptyStrings(); paths.removeDuplicates(false);
    if (paths.size() > 32) throw Error(-32602, "Choose at most 32 approved folders");
    for (auto path : paths) if (!juce::File::isAbsolutePath(path)) throw Error(-32602, "Approved folders must use absolute paths");
    return paths;
}
void ConnectionSettings::refreshClients(const String& selected)
{
    updating = true; juce::Array<EnumParameter::EnumValue> options;
    for (auto grant : permissions.list()) options.add({grant.label + " (" + grant.id.substring(0, 6) + ")", grant.id});
    clients->setOptions(options); clients->setValueWithData(selected); updating = false; selectClient();
}
void ConnectionSettings::selectClient()
{
    updating = true; newToken.clear(); tokenClient.clear(); copyToken->setEnabled(false); forgetToken->setEnabled(false);
    auto id = clients->getValueData().toString();
    for (auto* action : {save, rotate, revoke}) action->setEnabled(!id.isEmpty());
    for (auto grant : permissions.list()) if (grant.id == id)
    { clientName->setValue(grant.label); profile->setValueWithData((int) grant.access); advanced->setValue(grant.advancedExecution || grant.access == Access::full); folders->setValue(grant.folders.joinIntoString("\n")); }
    bool full = (int) profile->getValueData() == (int) Access::full; advanced->setEnabled(!full); folders->setEnabled(!full); updating = false;
}
void ConnectionSettings::onContainerParameterChanged(Parameter* value)
{
    if (updating) return;
    if (value == clients) selectClient();
    if (value == profile) { const bool full = (int) profile->getValueData() == 3; advanced->setEnabled(!full); folders->setEnabled(!full); if (full) advanced->setValue(true); }
}
void ConnectionSettings::onContainerTriggerTriggered(Trigger* trigger)
{
    try
    {
        auto id = clients->getValueData().toString();
        auto access = (Access) (int) profile->getValueData();
        if (trigger == create)
        {
            auto name = clientName->stringValue().trim(); if (name.isEmpty()) throw Error(-32602, "Enter a client name first");
            auto credential = permissions.create(name.substring(0, 100), access, advanced->boolValue(), approvedFolders());
            refreshClients(credential.first.id); newToken = credential.second; tokenClient = credential.first.id;
            copyToken->setEnabled(true); forgetToken->setEnabled(true); result->setValue("Client created. Copy its credential now; it cannot be recovered later.");
        }
        else if (trigger == save) { permissions.update(id, access, advanced->boolValue(), approvedFolders()); refreshClients(id); result->setValue("Remembered access updated."); }
        else if (trigger == rotate) { newToken = permissions.rotate(id); tokenClient = id; copyToken->setEnabled(true); forgetToken->setEnabled(true); result->setValue("Old credential revoked. Copy the replacement now."); }
        else if (trigger == revoke) { permissions.revoke(id); refreshClients(); result->setValue("Client revoked. Existing streams will close."); }
        else if (trigger == copyToken && id == tokenClient && !newToken.isEmpty()) { juce::SystemClipboard::copyTextToClipboard(newToken); result->setValue("Credential copied. Store it as CHATAIGNE_MCP_TOKEN for this client."); }
        else if (trigger == forgetToken) { newToken.clear(); tokenClient.clear(); copyToken->setEnabled(false); forgetToken->setEnabled(false); }
        else if (trigger == copyCodex)
        { juce::SystemClipboard::copyTextToClipboard("[mcp_servers.chataigne]\nurl = \"" + endpoint() + "\"\nbearer_token_env_var = \"CHATAIGNE_MCP_TOKEN\"\n"); result->setValue("Codex config copied. Set CHATAIGNE_MCP_TOKEN for this client."); }
        else if (trigger == copyClaude)
        {
            juce::SystemClipboard::copyTextToClipboard(juce::JSON::toString(object({{"mcpServers", object({{"chataigne", object({{"type", "http"}, {"url", endpoint()}, {"headers", object({{"Authorization", "Bearer ${CHATAIGNE_MCP_TOKEN}"}})}})}})}})));
            result->setValue("Claude Code config copied. Set CHATAIGNE_MCP_TOKEN for this client.");
        }
        else if (trigger == test) testConnection();
    }
    catch (const Error& error) { result->setValue(String(error.what())); }
}
void ConnectionSettings::timerCallback()
{
    String text; auto entries = server.activity();
    for (int i = juce::jmax(0, entries.size() - 20); i < entries.size(); ++i)
        text += entries[i].method + "  HTTP " + String(entries[i].status) + "  " + String(entries[i].milliseconds, 1) + " ms\n";
    if (text.isEmpty()) text = server.getError().isEmpty() ? "No client activity yet." : server.getError();
    activity->setValue(text);
}
void ConnectionSettings::testConnection()
{
    if (!server.isRunning()) { result->setValue("Enable the MCP listener first."); return; }
    if (newToken.isEmpty() || tokenClient != clients->getValueData().toString()) { result->setValue("Create or rotate a client to test its new credential."); return; }
    test->setEnabled(false); result->setValue("Testing authenticated connection...");
    auto url = endpoint(), token = newToken; juce::WeakReference<ControllableContainer> owner(this);
    juce::Thread::launch([owner, url, token]
    {
        auto request = object({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "server/discover"}, {"params", object({{"_meta", object({
            {"io.modelcontextprotocol/protocolVersion", "2026-07-28"}, {"io.modelcontextprotocol/clientInfo", object({{"name", "Connection test"}, {"version", "1"}})}, {"io.modelcontextprotocol/clientCapabilities", object()}})}})}});
        juce::WebInputStream stream(juce::URL(url).withPOSTData(juce::JSON::toString(request)), true);
        stream.withExtraHeaders("Authorization: Bearer " + token + "\r\nContent-Type: application/json\r\nAccept: application/json, text/event-stream\r\nMCP-Protocol-Version: 2026-07-28\r\nMcp-Method: server/discover").withConnectionTimeout(4000).withNumRedirectsToFollow(0);
        String message = "Connection failed. Check the port and listener status.";
        if (stream.connect(nullptr)) message = stream.getStatusCode() == 200 ? "Authenticated MCP connection succeeded." : "Connection rejected with HTTP " + String(stream.getStatusCode()) + ". Check the credential and access.";
        juce::MessageManager::callAsync([owner, message] { if (auto* settings = dynamic_cast<ConnectionSettings*>(owner.get())) { settings->test->setEnabled(true); settings->result->setValue(message); } });
    });
}
}
