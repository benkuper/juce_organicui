/* Optional, application-independent MCP operation and permission foundation. */
#pragma once
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>

namespace organic::mcp
{
using juce::var;
using juce::String;
var object(std::initializer_list<std::pair<juce::Identifier, var>> properties = {});
var array(std::initializer_list<var> values);
String secureToken();
String fingerprint(const String& token);
var parseBounded(const String& json);

struct Error : std::runtime_error
{
    int code;
    var data;
    Error(int code, const String& message, var data = {})
        : std::runtime_error(message.toStdString()), code(code), data(data) {}
};

enum class Access { inspect, control, edit, full };
enum Capability : unsigned { inspect = 1, control = 2, edit = 4, advanced = 8, files = 16 };
struct Grant
{
    String id, label, digest;
    Access access = Access::inspect;
    bool advancedExecution = false;
    juce::StringArray folders;
    bool allows(unsigned capabilities) const;
    bool allowsFile(const juce::File&) const;
};

// Only credential fingerprints are persisted. The bearer secret is displayed once.
class Permissions
{
public:
    explicit Permissions(juce::File store);
    std::pair<Grant, String> create(const String& label, Access, bool advanced = false,
                                   juce::StringArray folders = {});
    void update(const String& id, Access, bool advanced, juce::StringArray folders);
    String rotate(const String& id);
    void revoke(const String& id);
    Grant authenticate(const String& bearer) const;
    juce::Array<Grant> list() const;
private:
    juce::File store;
    mutable juce::CriticalSection lock;
    juce::Array<Grant> grants;
    void save() const;
};

struct Context
{
    Grant grant;
    std::shared_ptr<std::atomic<bool>> cancelled = std::make_shared<std::atomic<bool>>(false);
    double deadline = juce::Time::getMillisecondCounterHiRes() + 30000;
    juce::int64 generation = 0;
    void check() const;
    void require(unsigned capabilities) const;
    void requireFile(const juce::File&) const;
};

// Schemas use a deliberately bounded subset of JSON Schema. Unsupported validation
// keywords are rejected when registering an operation, never silently ignored.
void checkSchema(const var& schema, int depth = 0);
void validate(const var& schema, const var& value, const String& path = "$", int depth = 0);
var schema(const var& properties = object(), const var& required = array({}));
var stringSchema();
var integerSchema(int minimum, int maximum);

struct Operation
{
    String name, description;
    var input = schema(), output = schema();
    unsigned capabilities = inspect;
    bool reversible = false;
    bool externalEffects = false;
    std::function<void(const var&, const Context&)> prevalidate;
    std::function<var(const var&, Context&)> execute;
    var descriptor() const;
};
struct Resource
{
    String uri, name, description;
    std::function<var(Context&)> read;
};
struct Prompt
{
    String name, description, text;
};

class Registry
{
public:
    void add(Operation);
    void add(Resource);
    void add(Prompt);
    var tools(const String& cursor = {}) const;
    var resources(const String& cursor = {}) const;
    var prompts(const String& cursor = {}) const;
    const Operation& find(const String&) const;
    void prevalidate(const String& name, const var& arguments, const Context&) const;
    var call(const String& name, const var& arguments, Context&) const;
    var read(const String& uri, Context&) const;
    var prompt(const String& name) const;
private:
    std::map<String, Operation> operations;
    std::map<String, Resource> resourceMap;
    std::map<String, Prompt> promptMap;
};

struct HttpRequest
{
    String method = "POST", host, origin, authorization, version, session, rpcMethod, rpcName;
    String contentType = "application/json", accept = "application/json, text/event-stream";
    String body;
};
struct HttpReply
{
    int status = 200;
    var body;
    String session;
    bool stream = false;
    bool modern = false;
    var notifications;
    var requestId;
};

// Application access happens only through dispatch(), on the application thread.
// Modern requests are stateless; legacy session identity is bound to a credential.
class Protocol
{
public:
    Protocol(Registry&, Permissions&, String name, String version);
    HttpReply dispatch(const HttpRequest&, Context&);
    static var error(const var& id, int code, const String&, var data = {});
    static bool supported(const String&);
    int port = 42002;
    void clearSessions();
    Grant streamGrant(const HttpRequest&) const;
    var streamFilters(const HttpRequest&) const;
private:
    Registry& registry;
    Permissions& permissions;
    String appName, appVersion;
    struct Session { String credential, version; bool initialized = false; double touched = 0; juce::StringArray subscriptions; };
    std::map<String, Session> sessions;
    var capabilities() const;
};
}
