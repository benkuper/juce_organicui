#pragma once
#include "McpCore.h"
#include <juce_events/juce_events.h>
#include <deque>
#include <mutex>
#include <thread>

namespace organic::mcp
{
// The listener owns no model pointers. Pending operations run on the message thread.
class Server : private juce::AsyncUpdater, private juce::Timer
{
public:
    Server(Registry&, Permissions&, String name, String version);
    ~Server() override;
    juce::Result start(int port = 42002);
    void stop();
    void invalidateDocument();
    juce::int64 documentGeneration() const { return generation.load(); }
    bool isRunning() const { return running.load(); }
    String getError() const;
    struct Activity { String method; int status; double milliseconds; };
    juce::Array<Activity> activity() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
    Protocol protocol;
    Registry& registry;
    std::atomic<bool> running { false };
    std::atomic<juce::int64> generation { 1 };
    mutable std::mutex mutex;
    String lastError;
    juce::Array<Activity> history;
    struct Job
    {
        HttpRequest request;
        Context context;
        double received;
        std::function<void(HttpReply)> reply;
        std::function<void(std::shared_ptr<Job>, HttpReply)> openStream;
    };
    std::deque<std::shared_ptr<Job>> queue;
    void enqueue(std::shared_ptr<Job>);
    void handleAsyncUpdate() override;
    struct Stream
    {
        std::shared_ptr<Job> job;
        HttpReply subscription;
        std::map<String, String> hashes;
        std::function<void(const String&)> send;
        std::function<void()> close;
        std::shared_ptr<std::atomic<bool>> inFlight, closed;
        double lastSent = 0;
    };
    std::vector<Stream> streams;
    void timerCallback() override;
};
}
