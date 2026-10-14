#pragma once
#include "AssistantProvider.h"

namespace organic::assistant
{
// Hosts the user's unmodified Claude Code executable. Authentication belongs to
// that executable; this provider never reads or stores Anthropic credentials.
class ClaudeProvider final : public AgentProvider
{
public:
    struct ProcessResult { String output; int exitCode = 0; };
    using Runner = std::function<ProcessResult(const juce::StringArray&, const String&, std::shared_ptr<std::atomic<bool>>)>;
    ClaudeProvider(mcp::Registry&, mcp::Permissions&, juce::File storage, String appName,
                   std::function<juce::int64()> generation, std::function<var()> projectContext, Runner = {});
    ~ClaudeProvider() override { cancel(); }
    ProviderInfo info() const override { return {"claude", "Claude", "Connect Claude Code", "Connect your installed Claude Code account to get started.", "https://claude.ai/settings/usage", false}; }
    juce::StringArray efforts(const String& model) const override { return model == "opus" || model == "sonnet" ? juce::StringArray{"low", "medium", "high"} : juce::StringArray{}; }
    void signIn(bool) override;
    void refreshModels() override;
    void signOut() override;
private:
    Runner runner;
    ProcessResult run(juce::StringArray, const String& input = {});
    void connect(bool interactive);
    var accessToken(var record) override;
    void infer(const var&, const var&, ResponseStream&) override;
};
}
