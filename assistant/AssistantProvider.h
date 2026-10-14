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
    String active, conversation, transcript, status = "Sign in to get started.";
    Activity activity = Activity::idle;
    bool busy = false, signedIn = false;
};
// Other native providers can implement this interface without changing the docked UI.
class Provider
{
public:
    virtual ~Provider() = default;
    virtual View view() const = 0;
    virtual void signIn(bool newAccount) = 0;
    virtual void select(const String& key) = 0;
    virtual void refreshModels() = 0;
    virtual void send(const String& text, const String& model, const String& effort = {}) = 0;
    virtual void cancel() = 0;
    virtual void signOut() = 0;
    virtual void clearHistory() = 0;
    virtual void newConversation() = 0;
    virtual void selectConversation(const String&) = 0;
    virtual mcp::Grant grant() const = 0;
    virtual void setAccess(mcp::Access, bool advanced, juce::StringArray folders) = 0;
};

class ChatGptProvider final : public Provider, private juce::AsyncUpdater
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
    ChatGptProvider(mcp::Registry&, mcp::Permissions&, juce::File storage, String appName,
                    std::function<juce::int64()> generation, std::function<var()> projectContext, HostServices = {});
    ~ChatGptProvider() override;
    View view() const override;
    void signIn(bool newAccount) override;
    void select(const String&) override;
    void refreshModels() override;
    void send(const String&, const String& model, const String& effort = {}) override;
    void cancel() override;
    void signOut() override;
    void clearHistory() override;
    void newConversation() override;
    void selectConversation(const String&) override;
    mcp::Grant grant() const override;
    void setAccess(mcp::Access, bool, juce::StringArray) override;
private:
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
    void start(std::function<void()>);
    void check() const;
    void status(const String&, Activity = Activity::idle);
    void append(const String&);
    void loadRecords();
    void saveRecord(const var&);
    var current() const;
    var accessToken(var record);
    var request(const String& url, const String& method = "GET", const String& body = {},
                const String& bearer = {}, bool form = false, ResponseStream* stream = nullptr);
    var onMessageThread(std::function<var()>);
    void models(var record);
    void loadConversations(const String& account);
    String historyName(const String& account) const;
    void saveConversation(const String& account, const var& input, const String& title);
    juce::InterProcessLock accountLock() const;
};
}
