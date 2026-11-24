#pragma once
#include "AssistantCore.h"
#include <juce_events/juce_events.h>
#include <thread>
#include <mutex>
#include <future>
#include <deque>

namespace organic::assistant
{
enum class Activity { idle, connecting, thinking, responding, working, completed, failed, stopped };
struct View
{
    var accounts = mcp::array({}), models = mcp::array({}), conversations = mcp::array({});
    var pendingMessages = mcp::array({}), tasks = mcp::array({});
    String active, conversation, transcript, status = "Sign in to get started.";
    String recovery, lastModel, lastEffort;
    String workTitle, workDetail;
    Activity activity = Activity::idle;
    bool busy = false, signedIn = false, turnActive = false, queuePaused = false, canResume = false;
};
struct ProviderInfo
{
    String id, name, signInText, signInHint, usageUrl;
    bool multipleAccounts = true;
};
// Other native providers can implement this interface without changing the docked UI.
class Provider
{
public:
    virtual ~Provider() = default;
    virtual ProviderInfo info() const = 0;
    virtual juce::StringArray efforts(const String& model) const = 0;
    virtual View view() const = 0;
    virtual void signIn(bool newAccount) = 0;
    virtual void select(const String& key) = 0;
    virtual void refreshModels() = 0;
    virtual void send(const String& text, const String& model, const String& effort = {}) = 0;
    virtual void steer(const String& text) = 0;
    virtual void steerQueued(const String& id) = 0;
    virtual void removeQueued(const String& id) = 0;
    virtual void resume(const String& model = {}, const String& effort = {}, const String& guidance = {}) = 0;
    virtual void runQueue() = 0;
    virtual void cancel() = 0;
    virtual void signOut() = 0;
    virtual void clearHistory() = 0;
    virtual void newConversation() = 0;
    virtual void selectConversation(const String&) = 0;
    virtual mcp::Grant grant() const = 0;
    virtual void setAccess(mcp::Access, bool advanced, juce::StringArray folders) = 0;
};

// Shared conversation, completion, permissions and local tool execution loop.
// Providers implement authentication, model discovery and inference transport.
class AgentProvider : public Provider, private juce::AsyncUpdater
{
public:
    struct HostServices
    {
        // Optional injection for embedders and offline integration tests. Production
        // uses the fixed HTTPS endpoints and system browser when these are empty.
        std::function<var(const String&, const String&, const String&, const String&, bool,
            ResponseStream*, const std::function<void(const String&)>&, std::shared_ptr<std::atomic<bool>>)> request;
        std::function<bool(const juce::URL&)> openBrowser;
    };
    ~AgentProvider() override;
    View view() const override;
    void select(const String&) override;
    void send(const String&, const String& model, const String& effort = {}) override;
    void steer(const String&) override;
    void steerQueued(const String&) override;
    void removeQueued(const String&) override;
    void resume(const String& model = {}, const String& effort = {}, const String& guidance = {}) override;
    void runQueue() override;
    void cancel() override;
    void clearHistory() override;
    void newConversation() override;
    void selectConversation(const String&) override;
    mcp::Grant grant() const override;
    void setAccess(mcp::Access, bool, juce::StringArray) override;
protected:
    AgentProvider(mcp::Registry&, mcp::Permissions&, juce::File storage, String appName,
                  std::function<juce::int64()> generation, std::function<var()> projectContext, HostServices = {});
    mcp::Registry& registry;
    mcp::Permissions& permissions;
    ProtectedStore store;
    String appName, hostId;
    HostServices host;
    std::function<juce::int64()> generation;
    std::function<var()> projectContext;
    mutable std::mutex mutex, networkMutex;
    View state;
    var records = mcp::array({}), activeRecord;
    std::thread worker;
    std::shared_ptr<std::atomic<bool>> cancelled;
    std::shared_ptr<juce::WebInputStream> network;
    std::shared_ptr<juce::StreamingSocket> callback;
    struct Job { std::function<var()> run; std::promise<var> result; std::shared_ptr<std::atomic<bool>> cancelled; };
    std::deque<std::shared_ptr<Job>> queue;
    void handleAsyncUpdate() override;
    void start(std::function<void()>, bool turn = false);
    void check() const;
    void status(const String&, Activity = Activity::idle);
    void append(const String&);
    void loadRecords();
    void saveRecord(const var&);
    var current() const;
    virtual var accessToken(var record) = 0;
    virtual void infer(const var& payload, const var& record, ResponseStream&) = 0;
    var request(const String& url, const String& method = "GET", const String& body = {},
                const String& bearer = {}, bool form = false, ResponseStream* stream = nullptr);
    var onMessageThread(std::function<var()>);
    void loadConversations(const String& account);
    String historyName(const String& account) const;
    void saveConversation(const String& account, const var& input, const String& title);
    juce::InterProcessLock accountLock() const;
    void startTurn(const String&, const String& model, const String& effort, bool resuming = false);
    void validateMessage(const String&, const String& model, const String& effort) const;
    void savePending();
    void describeRecovery(const String& reason, bool error = false);
};

class ChatGptProvider final : public AgentProvider
{
public:
    ChatGptProvider(mcp::Registry&, mcp::Permissions&, juce::File storage, String appName,
                    std::function<juce::int64()> generation, std::function<var()> projectContext, HostServices = {});
    ProviderInfo info() const override { return {"chatgpt", "ChatGPT", "Continue with ChatGPT", "Sign in once with ChatGPT to get started.", "https://chatgpt.com/settings/usage", true}; }
    juce::StringArray efforts(const String& model) const override { return reasoningEfforts(model); }
    void signIn(bool newAccount) override;
    void refreshModels() override;
    void signOut() override;
private:
    var accessToken(var record) override;
    void infer(const var&, const var&, ResponseStream&) override;
    void models(var record);
};
}
