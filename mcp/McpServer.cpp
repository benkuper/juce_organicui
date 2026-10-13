#include "McpServer.h"
#ifndef USE_STANDALONE_ASIO
#define USE_STANDALONE_ASIO 1
#endif
#include <juce_simpleweb/webserver/server_http.hpp>
#include <future>

namespace organic::mcp
{
struct Server::Impl
{
    using Http = SimpleWeb::Server<SimpleWeb::HTTP>;
    Http listener;
    std::thread thread;
};
Server::Server(Registry& r, Permissions& p, String name, String version)
    : protocol(r, p, name, version), registry(r) {}
Server::~Server() { stop(); }
String Server::getError() const { std::lock_guard<std::mutex> guard(mutex); return lastError; }
juce::Array<Server::Activity> Server::activity() const { std::lock_guard<std::mutex> guard(mutex); return history; }
juce::Result Server::start(int port)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    stop();
    if (port < 1 || port > 65535) return juce::Result::fail("Invalid port");
    impl = std::make_unique<Impl>(); protocol.port = port;
    auto& listener = impl->listener;
    listener.config.address = "127.0.0.1";
    listener.config.port = static_cast<unsigned short>(port);
    listener.config.thread_pool_size = 2;
    listener.config.max_request_streambuf_size = 1024 * 1024 + 16384;
    listener.config.timeout_request = 5;
    listener.config.timeout_content = 35;
    auto handler = [this](std::shared_ptr<Impl::Http::Response> response, std::shared_ptr<Impl::Http::Request> request)
    {
        auto header = [&request](const char* name)
        {
            auto found = request->header.find(name);
            return found == request->header.end() ? String() : String(found->second);
        };
        auto job = std::make_shared<Job>();
        job->received = juce::Time::getMillisecondCounterHiRes();
        auto& r = job->request;
        r.method = request->method; r.host = header("Host"); r.origin = header("Origin");
        r.authorization = header("Authorization"); r.version = header("MCP-Protocol-Version");
        r.session = header("Mcp-Session-Id"); r.rpcMethod = header("Mcp-Method"); r.rpcName = header("Mcp-Name");
        r.accept = header("Accept"); r.contentType = header("Content-Type"); r.body = request->content.string();
        job->context.generation = generation.load();
        job->reply = [response](HttpReply reply)
        {
            SimpleWeb::CaseInsensitiveMultimap headers {{"Content-Type", "application/json"}, {"Cache-Control", "no-store"}, {"X-Content-Type-Options", "nosniff"}};
            if (reply.status == 401) headers.emplace("WWW-Authenticate", "Bearer realm=\"OrganicUI MCP\"");
            if (reply.status == 405) headers.emplace("Allow", "POST, DELETE");
            if (reply.session.isNotEmpty()) headers.emplace("Mcp-Session-Id", reply.session.toStdString());
            auto body = reply.body.isVoid() ? String() : juce::JSON::toString(reply.body, true);
            response->write(static_cast<SimpleWeb::StatusCode>(reply.status), body.toStdString(), headers);
        };
        job->openStream = [this, response](std::shared_ptr<Job> job, HttpReply reply)
        {
            if (streams.size() >= 32) { job->reply({429, Protocol::error({}, -32000, "Stream limit exceeded"), {}}); return; }
            Stream stream;
            stream.job = job; stream.subscription = reply;
            stream.inFlight = std::make_shared<std::atomic<bool>>(false);
            stream.closed = std::make_shared<std::atomic<bool>>(false);
            response->close_connection_after_response = true;
            *response << "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-store\r\nX-Accel-Buffering: no\r\nConnection: close\r\n\r\n";
            stream.send = [response, busy = stream.inFlight, closed = stream.closed](const String& data)
            {
                busy->store(true);
                *response << data.toStdString();
                response->send([busy, closed](const SimpleWeb::error_code& error) { if (error) closed->store(true); busy->store(false); });
            };
            if (reply.modern)
            {
                auto ack = object({{"jsonrpc", "2.0"}, {"method", "notifications/subscriptions/acknowledged"},
                    {"params", object({{"notifications", reply.notifications}, {"_meta", object({{"io.modelcontextprotocol/subscriptionId", reply.requestId}})}})}});
                stream.send("event: message\ndata: " + juce::JSON::toString(ack, true) + "\n\n");
            }
            else stream.send(": connected\n\n");
            stream.lastSent = juce::Time::getMillisecondCounterHiRes();
            streams.push_back(std::move(stream));
        };
        enqueue(job);
    };
    for (const auto* method : {"POST", "GET", "DELETE", "OPTIONS", "PUT", "PATCH"})
        listener.resource["^/mcp$"][method] = handler;
    for (const auto* method : {"POST", "GET", "DELETE", "OPTIONS", "PUT", "PATCH"})
        listener.default_resource[method] = [](auto response, auto) { response->write(SimpleWeb::StatusCode::client_error_not_found); };
    auto ready = std::make_shared<std::promise<String>>(); auto future = ready->get_future();
    impl->thread = std::thread([this, ready]
    {
        try { impl->listener.start([this, ready](unsigned short) { running = true; ready->set_value({}); }); }
        catch (const std::exception&) { try { ready->set_value("Cannot bind MCP listener. Check the selected port."); } catch (...) {} }
        running = false;
    });
    if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    { stop(); return juce::Result::fail("MCP listener timed out while starting"); }
    auto error = future.get();
    { std::lock_guard<std::mutex> guard(mutex); lastError = error; }
    if (error.isNotEmpty()) { stop(); return juce::Result::fail(error); }
    startTimer(200);
    return juce::Result::ok();
}
void Server::stop()
{
    running = false;
    stopTimer();
    for (auto& stream : streams) { stream.closed->store(true); stream.job->context.cancelled->store(true); }
    streams.clear();
    if (impl)
    {
        impl->listener.stop();
        if (impl->thread.joinable()) impl->thread.join();
    }
    cancelPendingUpdate();
    std::deque<std::shared_ptr<Job>> pending;
    { std::lock_guard<std::mutex> guard(mutex); pending.swap(queue); }
    for (auto& job : pending) { job->context.cancelled->store(true); job->reply({503, Protocol::error({}, -32000, "Server stopping"), {}}); }
    impl.reset(); protocol.clearSessions();
}
void Server::invalidateDocument() { ++generation; }
void Server::enqueue(std::shared_ptr<Job> job)
{
    // Cancellation may arrive while the message thread is busy. This touches
    // only credential records and queued envelopes, never application objects.
    var cancellation;
    if (job->request.method == "POST" && job->request.version != "2026-07-28")
        try { cancellation = parseBounded(job->request.body); } catch (...) {}
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (!running || queue.size() >= 64)
        {
            job->reply({running ? 429 : 503, Protocol::error({}, -32000, running ? "Request queue is full" : "Server unavailable"), {}});
            return;
        }
        if (cancellation["method"].toString() == "notifications/cancelled" && !cancellation.hasProperty("id")
            && job->request.authorization.startsWith("Bearer ")
            && (job->request.host == "127.0.0.1:" + String(protocol.port) || job->request.host == "localhost:" + String(protocol.port))
            && (job->request.origin.isEmpty() || job->request.origin == "http://" + job->request.host))
        {
            for (auto& pending : queue)
                if (pending->request.authorization == job->request.authorization && pending->request.session == job->request.session)
                    try
                    {
                        auto envelope = parseBounded(pending->request.body);
                        if (envelope.hasProperty("id") && envelope["id"] == cancellation["params"]["requestId"])
                            pending->context.cancelled->store(true);
                    }
                    catch (...) {}
        }
        queue.push_back(job);
    }
    triggerAsyncUpdate();
}
void Server::handleAsyncUpdate()
{
    // A bounded slice keeps UI and playback events responsive under load.
    auto until = juce::Time::getMillisecondCounterHiRes() + 8;
    for (int i = 0; i < 8; ++i)
    {
        std::shared_ptr<Job> job;
        { std::lock_guard<std::mutex> guard(mutex); if (queue.empty()) break; job = queue.front(); queue.pop_front(); }
        HttpReply reply;
        if (!running || job->context.generation != generation.load())
            reply = {409, Protocol::error({}, -32005, "Project changed while request was queued"), {}};
        else reply = protocol.dispatch(job->request, job->context);
        {
            std::lock_guard<std::mutex> guard(mutex);
            history.add({job->request.rpcMethod.isNotEmpty() ? job->request.rpcMethod : "legacy request", reply.status,
                juce::Time::getMillisecondCounterHiRes() - job->received});
            if (history.size() > 200) history.remove(0);
        }
        if (reply.stream) job->openStream(job, reply); else job->reply(reply);
        if (juce::Time::getMillisecondCounterHiRes() >= until) break;
    }
    { std::lock_guard<std::mutex> guard(mutex); if (!queue.empty()) triggerAsyncUpdate(); }
}
void Server::timerCallback()
{
    auto now = juce::Time::getMillisecondCounterHiRes();
    for (auto it = streams.begin(); it != streams.end();)
    {
        auto& stream = *it;
        if (stream.closed->load() || stream.job->context.generation != generation.load()
            || (stream.inFlight->load() && now - stream.lastSent > 5000))
        { stream.job->context.cancelled->store(true); it = streams.erase(it); continue; }
        try
        {
            // Reauthenticate every tick so rotation, revocation, and profile edits
            // apply to existing streams without waiting for a reconnect.
            stream.job->context.grant = protocol.streamGrant(stream.job->request);
            if (!stream.subscription.modern) stream.subscription.notifications = protocol.streamFilters(stream.job->request);
            stream.job->context.deadline = now + 1000;
            if (stream.inFlight->load()) { ++it; continue; }
            String data;
            auto uris = stream.subscription.notifications["resourceSubscriptions"];
            if (uris.isArray()) for (const auto& uri : *uris.getArray())
            {
                auto content = juce::JSON::toString(registry.read(uri.toString(), stream.job->context), true);
                auto hash = fingerprint(content);
                auto previous = stream.hashes.find(uri.toString());
                if (previous != stream.hashes.end() && previous->second != hash)
                {
                    auto params = object({{"uri", uri}});
                    if (stream.subscription.modern) params.getDynamicObject()->setProperty("_meta", object({{"io.modelcontextprotocol/subscriptionId", stream.subscription.requestId}}));
                    auto notification = object({{"jsonrpc", "2.0"}, {"method", "notifications/resources/updated"}, {"params", params}});
                    data += "event: message\ndata: " + juce::JSON::toString(notification, true) + "\n\n";
                }
                stream.hashes[uri.toString()] = hash;
            }
            if (data.isEmpty() && now - stream.lastSent >= 1000) data = ": keep-alive\n\n";
            if (data.isNotEmpty()) { stream.lastSent = now; stream.send(data); }
        }
        catch (...) { stream.closed->store(true); }
        ++it;
    }
}
}
