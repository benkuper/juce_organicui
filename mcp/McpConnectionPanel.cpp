#include "McpConnectionPanel.h"

namespace organic::mcp
{
ConnectionPanel::ConnectionPanel(const String& name, Permissions& p, Server& s,
    std::function<bool()> getEnabled, std::function<void(bool)> setEnabled,
    std::function<int()> getPort, std::function<void(int)> setPort)
    : ShapeShifterContentComponent(name), permissions(p), server(s), getEnabled(getEnabled), setEnabled(setEnabled), getPort(getPort), setPort(setPort)
{
    addAndMakeVisible(viewport); viewport.setViewedComponent(&form, false);
    for (auto* component : std::initializer_list<juce::Component*>{&title, &explanation, &endpoint, &profileLabel, &clientLabel, &foldersLabel, &result,
        &enabled, &advanced, &clients, &profiles, &clientName, &folders, &activity, &create, &save, &rotate, &revoke, &copyToken, &clearToken, &copyCodex, &copyClaude, &test, &tunnel, &port}) form.addAndMakeVisible(component);
    title.setText("MCP connections", juce::dontSendNotification);
    title.setFont(juce::Font(20.0f));
    explanation.setText("Create one credential per client. Access is remembered across projects and restarts. Advanced execution permits code and system commands; it is not a filesystem sandbox.", juce::dontSendNotification);
    explanation.setMinimumHorizontalScale(1.0f);
    clientLabel.setText("Client name", juce::dontSendNotification);
    profileLabel.setText("Remembered access profile", juce::dontSendNotification);
    foldersLabel.setText("Approved folders, one absolute path per line", juce::dontSendNotification);
    clientName.setTextToShowWhenEmpty("e.g. My desktop assistant", juce::Colours::grey);
    clientName.setName("Client name"); folders.setName("Approved folders"); profiles.setName("Access profile"); clients.setName("Clients"); result.setName("Connection result");
    folders.setMultiLine(true); folders.setReturnKeyStartsNewLine(true);
    activity.setMultiLine(true); activity.setReadOnly(true); activity.setScrollbarsShown(true);
    profiles.addItemList({"Inspect", "Control", "Edit", "Full Access"}, 1); profiles.setSelectedId(1);
    profiles.onChange = [this]
    {
        const bool full = access() == Access::full;
        advanced.setEnabled(!full); folders.setEnabled(!full);
        if (full) advanced.setToggleState(true, juce::dontSendNotification);
    };
    port.setRange(1, 65535, 1); port.setSliderStyle(juce::Slider::IncDecButtons); port.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 90, 24);
    port.setValue(getPort(), juce::dontSendNotification);
    port.onValueChange = [this] { this->setPort((int) port.getValue()); };
    enabled.onClick = [this] { this->setEnabled(enabled.getToggleState()); timerCallback(); };
    clients.onChange = [this] { selectClient(); };
    create.onClick = [this] { perform([&]
    {
        if (clientName.getText().trim().isEmpty()) throw Error(-32602, "Enter a client name first.");
        auto credential = permissions.create(clientName.getText().trim().substring(0, 100), access(), advanced.getToggleState(), approvedFolders());
        refreshClients(credential.first.id); newToken = credential.second; newTokenClient = credential.first.id;
        result.setText("Client created. Copy the new credential now; it cannot be recovered later.", juce::dontSendNotification);
        copyToken.setEnabled(true); clearToken.setEnabled(true);
    }); };
    save.onClick = [this] { perform([&] { auto id = selectedId(); if (id.isEmpty()) throw Error(-32602, "Select a client first."); permissions.update(id, access(), advanced.getToggleState(), approvedFolders()); refreshClients(id); result.setText("Remembered access updated.", juce::dontSendNotification); }); };
    rotate.onClick = [this] { perform([&] { newToken = permissions.rotate(selectedId()); newTokenClient = selectedId(); copyToken.setEnabled(true); clearToken.setEnabled(true); result.setText("Previous credential revoked. Copy the replacement now.", juce::dontSendNotification); }); };
    revoke.onClick = [this] { perform([&] { permissions.revoke(selectedId()); refreshClients(); result.setText("Client revoked. Existing streams will close.", juce::dontSendNotification); }); };
    copyToken.onClick = [this] { if (newToken.isNotEmpty() && newTokenClient == selectedId()) { juce::SystemClipboard::copyTextToClipboard(newToken); result.setText("Credential copied. Store it as CHATAIGNE_MCP_TOKEN for your client.", juce::dontSendNotification); } };
    clearToken.onClick = [this] { newToken.clear(); newTokenClient.clear(); copyToken.setEnabled(false); clearToken.setEnabled(false); };
    copyCodex.onClick = [this]
    {
        juce::SystemClipboard::copyTextToClipboard("[mcp_servers.chataigne]\nurl = \"" + endpointURL() + "\"\nbearer_token_env_var = \"CHATAIGNE_MCP_TOKEN\"\n");
        result.setText("Codex configuration copied. Set CHATAIGNE_MCP_TOKEN to this client's credential.", juce::dontSendNotification);
    };
    copyClaude.onClick = [this]
    {
        auto config = object({{"mcpServers", object({{"chataigne", object({{"type", "http"}, {"url", endpointURL()},
            {"headers", object({{"Authorization", "Bearer ${CHATAIGNE_MCP_TOKEN}"}})}})}})}});
        juce::SystemClipboard::copyTextToClipboard(juce::JSON::toString(config));
        result.setText("Claude Code configuration copied. Set CHATAIGNE_MCP_TOKEN to this client's credential.", juce::dontSendNotification);
    };
    test.onClick = [this] { testConnection(); };
    tunnel.onClick = [] { juce::URL("https://developers.openai.com/api/docs/guides/secure-mcp-tunnels").launchInDefaultBrowser(); };
    refreshClients(); timerCallback(); startTimer(500);
}
ConnectionPanel::~ConnectionPanel() { stopTimer(); viewport.setViewedComponent(nullptr, false); }
Access ConnectionPanel::access() const { return static_cast<Access>(juce::jlimit(0, 3, profiles.getSelectedId() - 1)); }
String ConnectionPanel::selectedId() const { int index = clients.getSelectedItemIndex(); return juce::isPositiveAndBelow(index, grants.size()) ? grants[index].id : String(); }
String ConnectionPanel::endpointURL() const { return "http://127.0.0.1:" + String(getPort()) + "/mcp"; }
juce::StringArray ConnectionPanel::approvedFolders() const
{
    juce::StringArray paths;
    for (const auto& line : juce::StringArray::fromLines(folders.getText()))
    {
        auto path = line.trim(); if (path.isEmpty()) continue;
        if (!juce::File::isAbsolutePath(path)) throw Error(-32602, "Approved folders must use absolute paths.");
        paths.addIfNotAlreadyThere(juce::File(path).getFullPathName());
    }
    if (paths.size() > 32) throw Error(-32602, "Choose at most 32 approved folders.");
    return paths;
}
void ConnectionPanel::perform(const std::function<void()>& action)
{
    try { action(); } catch (const Error& e) { result.setText(e.what(), juce::dontSendNotification); }
}
void ConnectionPanel::refreshClients(const String& selected)
{
    grants = permissions.list(); clients.clear(juce::dontSendNotification);
    int selectedIndex = 0;
    for (int i = 0; i < grants.size(); ++i) { clients.addItem(grants[i].label, i + 1); if (grants[i].id == selected) selectedIndex = i + 1; }
    clients.setSelectedId(selectedIndex, juce::dontSendNotification); selectClient();
}
void ConnectionPanel::selectClient()
{
    newToken.clear(); newTokenClient.clear(); copyToken.setEnabled(false); clearToken.setEnabled(false);
    int index = clients.getSelectedItemIndex(); bool selected = juce::isPositiveAndBelow(index, grants.size());
    save.setEnabled(selected); rotate.setEnabled(selected); revoke.setEnabled(selected);
    if (!selected) return;
    const auto& grant = grants[index]; clientName.setText(grant.label, false);
    profiles.setSelectedId((int) grant.access + 1, juce::dontSendNotification);
    advanced.setToggleState(grant.advancedExecution || grant.access == Access::full, juce::dontSendNotification);
    folders.setText(grant.folders.joinIntoString("\n"), false); profiles.onChange();
}
void ConnectionPanel::timerCallback()
{
    enabled.setToggleState(getEnabled(), juce::dontSendNotification);
    port.setValue(getPort(), juce::dontSendNotification);
    endpoint.setText((server.isRunning() ? "Listening: " : "Endpoint: ") + endpointURL(), juce::dontSendNotification);
    auto entries = server.activity(); String text;
    for (int i = juce::jmax(0, entries.size() - 20); i < entries.size(); ++i)
        text += entries[i].method + "  HTTP " + String(entries[i].status) + "  " + String(entries[i].milliseconds, 1) + " ms\n";
    if (text.isEmpty()) text = server.getError().isNotEmpty() ? server.getError() : "No client activity yet.";
    if (activity.getText() != text) activity.setText(text, false);
}
void ConnectionPanel::testConnection()
{
    if (!server.isRunning()) { result.setText("Enable the MCP listener first.", juce::dontSendNotification); return; }
    if (newToken.isEmpty() || newTokenClient != selectedId()) { result.setText("Create or rotate a client to test its new credential.", juce::dontSendNotification); return; }
    test.setEnabled(false); result.setText("Testing authenticated connection...", juce::dontSendNotification);
    auto url = endpointURL(), token = newToken;
    juce::Component::SafePointer<ConnectionPanel> safeThis(this);
    juce::Thread::launch([safeThis, url, token]
    {
        auto request = object({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "server/discover"}, {"params", object({{"_meta", object({
            {"io.modelcontextprotocol/protocolVersion", "2026-07-28"}, {"io.modelcontextprotocol/clientInfo", object({{"name", "Connection test"}, {"version", "1"}})}, {"io.modelcontextprotocol/clientCapabilities", object()}})}})}});
        juce::WebInputStream stream(juce::URL(url).withPOSTData(juce::JSON::toString(request)), true);
        stream.withExtraHeaders("Authorization: Bearer " + token + "\r\nContent-Type: application/json\r\nAccept: application/json, text/event-stream\r\nMCP-Protocol-Version: 2026-07-28\r\nMcp-Method: server/discover\r\n").withConnectionTimeout(4000).withNumRedirectsToFollow(0);
        String message = "Connection failed. Check the port and listener status.";
        if (stream.connect(nullptr)) message = stream.getStatusCode() == 200 ? "Authenticated MCP connection succeeded." : "Connection rejected with HTTP " + String(stream.getStatusCode()) + ". Check the client's credential and access.";
        juce::MessageManager::callAsync([safeThis, message] { if (safeThis) { safeThis->test.setEnabled(true); safeThis->result.setText(message, juce::dontSendNotification); } });
    });
}
void ConnectionPanel::resized()
{
    viewport.setBounds(getLocalBounds());
    int width = juce::jmax(420, viewport.getWidth() - 18); form.setSize(width, 730);
    auto bounds = form.getLocalBounds().reduced(12);
    title.setBounds(bounds.removeFromTop(30)); explanation.setBounds(bounds.removeFromTop(64));
    auto row = bounds.removeFromTop(28); enabled.setBounds(row.removeFromLeft(width - 160)); port.setBounds(row);
    endpoint.setBounds(bounds.removeFromTop(30)); clients.setBounds(bounds.removeFromTop(28)); bounds.removeFromTop(8);
    clientLabel.setBounds(bounds.removeFromTop(22)); clientName.setBounds(bounds.removeFromTop(26));
    profileLabel.setBounds(bounds.removeFromTop(24)); row = bounds.removeFromTop(28); profiles.setBounds(row.removeFromLeft((width - 24) / 2).reduced(0, 1)); advanced.setBounds(row);
    foldersLabel.setBounds(bounds.removeFromTop(25)); folders.setBounds(bounds.removeFromTop(80)); bounds.removeFromTop(8);
    auto buttons = [&](std::initializer_list<juce::Button*> controls)
    { auto line = bounds.removeFromTop(32); int cell = line.getWidth() / (int) controls.size(); for (auto* button : controls) button->setBounds(line.removeFromLeft(cell).reduced(2)); };
    buttons({&create, &save, &rotate, &revoke}); buttons({&copyToken, &clearToken}); buttons({&copyCodex, &copyClaude}); buttons({&test, &tunnel});
    result.setBounds(bounds.removeFromTop(52)); result.setMinimumHorizontalScale(1.0f);
    activity.setBounds(bounds.reduced(0, 4));
}
}
