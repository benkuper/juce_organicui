#pragma once
#include "McpServer.h"

namespace organic::mcp
{
class ConnectionPanel : public ShapeShifterContentComponent, private juce::Timer
{
public:
    ConnectionPanel(const String& name, Permissions&, Server&,
        std::function<bool()> getEnabled, std::function<void(bool)> setEnabled,
        std::function<int()> getPort, std::function<void(int)> setPort);
    ~ConnectionPanel() override;
    void resized() override;
private:
    Permissions& permissions;
    Server& server;
    std::function<bool()> getEnabled;
    std::function<void(bool)> setEnabled;
    std::function<int()> getPort;
    std::function<void(int)> setPort;
    juce::Viewport viewport;
    juce::Component form;
    juce::Label title, explanation, endpoint, profileLabel, clientLabel, foldersLabel, result;
    juce::ToggleButton enabled {"Enable authenticated MCP"}, advanced {"Advanced execution"};
    juce::ComboBox clients, profiles;
    juce::TextEditor clientName, folders, activity;
    juce::TextButton create {"Create client"}, save {"Save access"}, rotate {"Rotate credential"}, revoke {"Revoke client"};
    juce::TextButton copyToken {"Copy new credential"}, clearToken {"Forget displayed credential"};
    juce::TextButton copyCodex {"Copy Codex config"}, copyClaude {"Copy Claude Code config"}, test {"Test connection"}, tunnel {"ChatGPT tunnel setup"};
    juce::Slider port;
    juce::Array<Grant> grants;
    String newToken, newTokenClient;
    void timerCallback() override;
    void refreshClients(const String& selected = {});
    void selectClient();
    juce::StringArray approvedFolders() const;
    Access access() const;
    String selectedId() const;
    String endpointURL() const;
    void perform(const std::function<void()>&);
    void testConnection();
};
}
