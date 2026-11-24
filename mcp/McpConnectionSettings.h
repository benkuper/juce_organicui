#pragma once
#include "McpServer.h"
namespace organic::mcp
{
// Persistent settings model shared by the Global Settings editor and docked UI.
// Only a newly issued bearer is held privately until copied/forgotten; it is
// never a Controllable value, project field, log entry, or saved UI preference.
class ConnectionSettings final : public ControllableContainer, private juce::Timer
{
public:
    ConnectionSettings(Permissions&, Server&, std::function<int()> port);
    ~ConnectionSettings() override;
    Permissions& permissions;
    Server& server;
    EnumParameter *clients, *profile;
    BoolParameter* advanced;
    StringParameter *clientName, *folders, *result, *activity;
    Trigger *create, *save, *rotate, *revoke, *copyToken, *forgetToken, *copyCodex, *copyClaude, *test;
    void refreshClients(const String& selected = {});
    String endpoint() const;
    void onContainerParameterChanged(Parameter*) override;
    void onContainerTriggerTriggered(Trigger*) override;
private:
    std::function<int()> port;
    bool updating = false;
    String newToken, tokenClient;
    void selectClient();
    void timerCallback() override;
    void testConnection();
    juce::StringArray approvedFolders() const;
};
}
