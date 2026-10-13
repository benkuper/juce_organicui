#include "McpCore.h"
#include <cmath>
#if JUCE_WINDOWS
#include <filesystem>
#else
#include <cerrno>
#include <cstdlib>
#include <sys/stat.h>
#endif
#include <random>

namespace organic::mcp
{
var object(std::initializer_list<std::pair<juce::Identifier, var>> properties)
{
    auto* result = new juce::DynamicObject();
    for (const auto& p : properties) result->setProperty(p.first, p.second);
    return result;
}
var array(std::initializer_list<var> values)
{
    juce::Array<var> result;
    for (const auto& v : values) result.add(v);
    return result;
}
String secureToken()
{
    // random_device uses the operating system entropy source on supported targets.
    std::random_device entropy;
    unsigned char bytes[32];
    for (auto& b : bytes) b = static_cast<unsigned char>(entropy());
    return juce::String::toHexString(bytes, sizeof(bytes), 0);
}
String fingerprint(const String& token)
{
    return juce::SHA256(token.toRawUTF8(), token.getNumBytesAsUTF8()).toHexString();
}
var parseBounded(const String& json)
{
    if (json.getNumBytesAsUTF8() > 1024 * 1024) throw Error(-32600, "Payload limit exceeded");
    int depth = 0; bool quoted = false, escaped = false;
    for (auto character : json)
    {
        if (quoted)
        {
            if (escaped) escaped = false;
            else if (character == '\\') escaped = true;
            else if (character == '"') quoted = false;
        }
        else if (character == '"') quoted = true;
        else if (character == '{' || character == '[') { if (++depth > 32) throw Error(-32600, "JSON nesting limit exceeded"); }
        else if (character == '}' || character == ']') --depth;
    }
    var message;
    if (juce::JSON::parse(json, message).failed()) throw Error(-32700, "Parse error");
    return message;
}
bool Grant::allows(unsigned caps) const
{
    unsigned available = inspect;
    if (!folders.isEmpty()) available |= files;
    if (access >= Access::control) available |= control;
    if (access >= Access::edit) available |= edit | files;
    if (advancedExecution || access == Access::full) available |= advanced;
    return (caps & available) == caps;
}
static String canonicalPath(juce::File file)
{
    juce::StringArray suffix;
    for (int depth = 0; depth < 256; ++depth)
    {
#if JUCE_WINDOWS
        auto path = std::filesystem::u8path(file.getFullPathName().toStdString());
        std::error_code error; auto status = std::filesystem::symlink_status(path, error);
        if (error && error != std::errc::no_such_file_or_directory) return {};
        if (std::filesystem::exists(status) || std::filesystem::is_symlink(status))
        {
            auto resolved = std::filesystem::canonical(path, error);
            if (error) return {};
            auto result = juce::File(String(resolved.u8string()));
#else
        struct stat status {};
        if (::lstat(file.getFullPathName().toRawUTF8(), &status) == 0)
        {
            // Resolving an existing symlink must succeed. In particular, never
            // treat a dangling link as a harmless, not-yet-created file path.
            char* resolved = ::realpath(file.getFullPathName().toRawUTF8(), nullptr);
            if (!resolved) return {};
            auto result = juce::File(String::fromUTF8(resolved)); std::free(resolved);
#endif
            for (int i = suffix.size(); --i >= 0;) result = result.getChildFile(suffix[i]);
            return result.getFullPathName();
        }
#if !JUCE_WINDOWS
        if (errno != ENOENT) return {};
#endif
        auto parent = file.getParentDirectory();
        if (parent == file) return {};
        suffix.add(file.getFileName()); file = parent;
    }
    return {};
}
bool Grant::allowsFile(const juce::File& file) const
{
    if (access == Access::full) return true;
    if (!allows(files)) return false;
    auto target = canonicalPath(file); if (target.isEmpty()) return false;
    for (const auto& folder : folders)
    {
        auto root = canonicalPath(juce::File(folder)); if (root.isEmpty()) continue;
        auto prefix = root.endsWith(juce::File::getSeparatorString()) ? root : root + juce::File::getSeparatorString();
#if JUCE_WINDOWS
        if (target.equalsIgnoreCase(root) || target.startsWithIgnoreCase(prefix)) return true;
#else
        if (target == root || target.startsWith(prefix)) return true;
#endif
    }
    return false;
}
Permissions::Permissions(juce::File file) : store(file)
{
    if (!store.existsAsFile()) return;
    auto data = juce::JSON::parse(store);
    if (!data.isArray()) throw Error(-32603, "Invalid MCP credential store; access is disabled");
    for (const auto& row : *data.getArray())
    {
        if (row.getDynamicObject() == nullptr || !row["id"].isString() || row["digest"].toString().length() != 64
            || !row["access"].isInt() || (int) row["access"] < 0 || (int) row["access"] > 3)
            throw Error(-32603, "Invalid MCP credential store; access is disabled");
        Grant grant;
        grant.id = row["id"].toString(); grant.label = row["label"].toString();
        grant.digest = row["digest"].toString(); grant.access = static_cast<Access>((int) row["access"]);
        grant.advancedExecution = (bool) row["advanced"];
        if (auto* folders = row["folders"].getArray()) for (const auto& f : *folders) grant.folders.add(f.toString());
        grants.add(grant);
    }
}
void Permissions::save() const
{
    juce::Array<var> rows;
    for (const auto& g : grants)
    {
        juce::Array<var> folders; for (const auto& f : g.folders) folders.add(f);
        rows.add(object({{"id", g.id}, {"label", g.label}, {"digest", g.digest},
                         {"access", (int) g.access}, {"advanced", g.advancedExecution}, {"folders", folders}}));
    }
    if (store.getParentDirectory().createDirectory().failed()) throw Error(-32603, "Cannot create MCP credential store");
    juce::TemporaryFile temporary(store);
    if (!temporary.getFile().replaceWithText(juce::JSON::toString(rows)) || !temporary.overwriteTargetFileWithTemporary())
        throw Error(-32603, "Cannot persist MCP credentials");
}
std::pair<Grant, String> Permissions::create(const String& label, Access access, bool advancedGrant, juce::StringArray folders)
{
    const juce::ScopedLock guard(lock);
    auto token = secureToken();
    Grant grant { juce::Uuid().toString(), label, fingerprint(token), access, advancedGrant, folders };
    grants.add(grant);
    try { save(); } catch (...) { grants.removeLast(); throw; }
    return {grant, token};
}
void Permissions::update(const String& id, Access access, bool advancedGrant, juce::StringArray folders)
{
    const juce::ScopedLock guard(lock);
    for (auto& grant : grants) if (grant.id == id)
    {
        auto previous = grant;
        grant.access = access; grant.advancedExecution = advancedGrant; grant.folders = folders;
        try { save(); } catch (...) { grant = previous; throw; }
        return;
    }
    throw Error(-32602, "Unknown credential");
}
String Permissions::rotate(const String& id)
{
    const juce::ScopedLock guard(lock);
    for (auto& grant : grants) if (grant.id == id)
    {
        auto previous = grant.digest, token = secureToken(); grant.digest = fingerprint(token);
        try { save(); } catch (...) { grant.digest = previous; throw; }
        return token;
    }
    throw Error(-32602, "Unknown credential");
}
void Permissions::revoke(const String& id)
{
    const juce::ScopedLock guard(lock);
    auto previous = grants;
    for (int i = grants.size(); --i >= 0;) if (grants[i].id == id) grants.remove(i);
    try { save(); } catch (...) { grants = previous; throw; }
}
Grant Permissions::authenticate(const String& bearer) const
{
    const juce::ScopedLock guard(lock);
    if (bearer.length() != 64 || bearer.containsAnyOf("\r\n")) throw Error(-32001, "Authentication required");
    auto digest = fingerprint(bearer);
    for (const auto& g : grants)
    {
        unsigned difference = 0;
        for (int i = 0; i < 64; ++i) difference |= (unsigned) (g.digest[i] ^ digest[i]);
        if (difference == 0) return g;
    }
    throw Error(-32001, "Authentication required");
}
juce::Array<Grant> Permissions::list() const { const juce::ScopedLock guard(lock); return grants; }
void Context::check() const
{
    if (cancelled->load()) throw Error(-32800, "Request cancelled");
    if (juce::Time::getMillisecondCounterHiRes() > deadline) throw Error(-32002, "Request deadline exceeded");
}
void Context::require(unsigned caps) const
{
    requiredCapabilities |= caps;
    check(); if (!grant.allows(caps)) throw Error(-32003, "Access profile does not permit this operation");
}
void Context::requireFile(const juce::File& file) const
{
    check(); if (!grant.allowsFile(file)) throw Error(-32003, "Path is outside the approved folders");
    requiredPaths.addIfNotAlreadyThere(file.getFullPathName());
}
var schema(const var& properties, const var& required)
{
    return object({{"type", "object"}, {"properties", properties}, {"required", required}, {"additionalProperties", false}});
}
var stringSchema() { return object({{"type", "string"}, {"maxLength", 16384}}); }
var integerSchema(int minimum, int maximum)
{
    return object({{"type", "integer"}, {"minimum", minimum}, {"maximum", maximum}});
}
var anySchema() { return object(); }
var openObjectSchema() { auto s = schema(); s.getDynamicObject()->setProperty("additionalProperties", true); return s; }
void checkSchema(const var& s, int depth)
{
    if (depth > 24 || s.getDynamicObject() == nullptr) throw Error(-32603, "Invalid operation schema");
    if (s.getDynamicObject()->getProperties().isEmpty()) return;
    static const juce::StringArray supported {"type", "properties", "required", "additionalProperties", "items",
        "enum", "minimum", "maximum", "minItems", "maxItems", "minLength", "maxLength", "description", "title", "default"};
    for (const auto& p : s.getDynamicObject()->getProperties())
        if (!supported.contains(p.name.toString())) throw Error(-32603, "Unsupported schema keyword: " + p.name.toString());
    auto type = s["type"].toString();
    if (!juce::StringArray{"object", "array", "string", "number", "integer", "boolean", "null"}.contains(type))
        throw Error(-32603, "A schema must declare a supported type");
    if (type == "object")
    {
        if (s["properties"].getDynamicObject() == nullptr || !s["required"].isArray() || !s["additionalProperties"].isBool())
            throw Error(-32603, "Object schemas must declare properties, required, and additionalProperties");
        for (const auto& p : s["properties"].getDynamicObject()->getProperties()) checkSchema(p.value, depth + 1);
        for (const auto& r : *s["required"].getArray())
            if (!r.isString() || !s["properties"].hasProperty(r.toString())) throw Error(-32603, "Invalid required property");
    }
    if (type == "array") checkSchema(s["items"], depth + 1);
}
void validate(const var& s, const var& v, const String& path, int depth)
{
    if (depth > 24) throw Error(-32602, "Arguments exceed nesting limit");
    if (s.getDynamicObject() == nullptr) throw Error(-32603, "Invalid operation schema");
    if (s.getDynamicObject()->getProperties().isEmpty()) return;
    auto fail = [&path](const String& why) { throw Error(-32602, path + ": " + why); };
    const auto type = s["type"].toString();
    bool numeric = v.isInt() || v.isInt64() || v.isDouble();
    bool matches = (type == "object" && v.getDynamicObject() != nullptr) || (type == "array" && v.isArray())
        || (type == "string" && v.isString()) || (type == "boolean" && v.isBool())
        || (type == "number" && numeric) || (type == "integer" && numeric && std::floor((double) v) == (double) v)
        || (type == "null" && (v.isVoid() || v.isUndefined()));
    if (!matches) fail("expected " + type);
    if (numeric && (!std::isfinite((double) v) || (s.hasProperty("minimum") && (double) v < (double) s["minimum"])
        || (s.hasProperty("maximum") && (double) v > (double) s["maximum"]))) fail("number outside permitted range");
    if (auto* choices = s["enum"].getArray()) if (!choices->contains(v)) fail("unsupported value");
    if (v.isString())
    {
        auto length = v.toString().length();
        if ((s.hasProperty("minLength") && length < (int) s["minLength"])
            || (s.hasProperty("maxLength") && length > (int) s["maxLength"])) fail("string length outside permitted range");
    }
    if (v.isArray())
    {
        auto size = v.size();
        if ((s.hasProperty("minItems") && size < (int) s["minItems"])
            || (s.hasProperty("maxItems") && size > (int) s["maxItems"])) fail("array size outside permitted range");
        for (int i = 0; i < size; ++i) validate(s["items"], v[i], path + "[" + String(i) + "]", depth + 1);
    }
    if (type == "object")
    {
        for (const auto& required : *s["required"].getArray()) if (!v.hasProperty(required.toString())) fail("missing " + required.toString());
        auto properties = s["properties"];
        for (const auto& p : v.getDynamicObject()->getProperties())
        {
            if (properties.hasProperty(p.name)) validate(properties[p.name], p.value, path + "." + p.name.toString(), depth + 1);
            else if (!(bool) s["additionalProperties"]) fail("unknown property " + p.name.toString());
        }
    }
}
var Operation::descriptor() const
{
    return object({{"name", name}, {"description", description}, {"inputSchema", input}, {"outputSchema", output},
        {"annotations", object({{"readOnlyHint", capabilities == inspect}, {"destructiveHint", !reversible && capabilities != inspect},
            {"openWorldHint", externalEffects}})},
        {"_meta", object({{"org.organicui/requiredCapabilities", (int) capabilities}, {"org.organicui/reversible", reversible}})}});
}
void Registry::add(Operation op)
{
    if (!op.execute || operations.count(op.name)) throw Error(-32603, "Invalid or duplicate operation: " + op.name);
    checkSchema(op.input); checkSchema(op.output); operations.emplace(op.name, std::move(op));
}
void Registry::add(Resource r)
{
    if (!r.read || resourceMap.count(r.uri)) throw Error(-32603, "Invalid or duplicate resource");
    resourceMap.emplace(r.uri, std::move(r));
}
void Registry::add(Prompt p)
{
    if (promptMap.count(p.name)) throw Error(-32603, "Duplicate prompt");
    promptMap.emplace(p.name, std::move(p));
}
template<class Map, class Describe>
static var page(const Map& map, const String& cursor, const String& key, Describe describe)
{
    juce::Array<var> rows;
    auto it = cursor.isEmpty() ? map.begin() : map.upper_bound(cursor);
    String last;
    for (int count = 0; it != map.end() && count < 100; ++it, ++count) { rows.add(describe(it->second)); last = it->first; }
    auto result = object({{juce::Identifier(key), rows}});
    if (it != map.end()) result.getDynamicObject()->setProperty("nextCursor", last);
    return result;
}
var Registry::tools(const String& cursor) const { return page(operations, cursor, "tools", [](const auto& op) { return op.descriptor(); }); }
var Registry::resources(const String& cursor) const
{
    return page(resourceMap, cursor, "resources", [](const auto& r) { return object({{"uri", r.uri}, {"name", r.name}, {"description", r.description}, {"mimeType", "application/json"}}); });
}
var Registry::prompts(const String& cursor) const
{
    return page(promptMap, cursor, "prompts", [](const auto& p) { return object({{"name", p.name}, {"description", p.description}}); });
}
const Operation& Registry::find(const String& name) const
{
    auto it = operations.find(name); if (it == operations.end()) throw Error(-32602, "Unknown tool: " + name); return it->second;
}
void Registry::prevalidate(const String& name, const var& args, const Context& context) const
{
    const auto& op = find(name); context.require(op.capabilities); validate(op.input, args);
    if (op.prevalidate) op.prevalidate(args, context);
}
static thread_local Context* activeContext = nullptr;
Context* Registry::executingContext() { return activeContext; }
var Registry::call(const String& name, const var& args, Context& context) const
{
    struct Scope
    {
        Context*& current; Context* previous;
        Scope(Context*& slot, Context& next) : current(slot), previous(slot) { if (!previous) { next.requiredCapabilities = 0; next.requiredPaths.clear(); } current = &next; }
        ~Scope() { current = previous; }
    } scope(activeContext, context);
    prevalidate(name, args, context);
    auto result = find(name).execute(args, context);
    try { validate(find(name).output, result); }
    catch (const Error&) { throw Error(-32603, "Provider returned an invalid result", object({{"operation", name}, {"mayHaveExecuted", true}})); }
    return result;
}
var Registry::read(const String& uri, Context& c) const
{
    c.require(inspect); auto it = resourceMap.find(uri); if (it == resourceMap.end()) throw Error(-32004, "Unknown resource");
    return object({{"contents", array({object({{"uri", uri}, {"mimeType", "application/json"}, {"text", juce::JSON::toString(it->second.read(c))}})})}});
}
var Registry::prompt(const String& name) const
{
    auto it = promptMap.find(name); if (it == promptMap.end()) throw Error(-32602, "Unknown prompt");
    return object({{"description", it->second.description}, {"messages", array({object({{"role", "user"}, {"content", object({{"type", "text"}, {"text", it->second.text}})}})})}});
}
Protocol::Protocol(Registry& r, Permissions& p, String name, String version)
    : registry(r), permissions(p), appName(name), appVersion(version) {}
bool Protocol::supported(const String& v) { return v == "2026-07-28" || v == "2025-11-25" || v == "2025-06-18"; }
void Protocol::clearSessions() { sessions.clear(); }
Grant Protocol::streamGrant(const HttpRequest& request) const
{
    auto grant = permissions.authenticate(request.authorization.substring(7));
    if (request.version != "2026-07-28")
    {
        auto it = sessions.find(request.session);
        if (it == sessions.end() || it->second.credential != grant.digest || !it->second.initialized)
            throw Error(-32001, "Session expired");
    }
    return grant;
}
var Protocol::streamFilters(const HttpRequest& request) const
{
    juce::Array<var> uris;
    auto it = sessions.find(request.session);
    if (it != sessions.end()) for (const auto& uri : it->second.subscriptions) uris.add(uri);
    return object({{"resourceSubscriptions", uris}});
}
var Protocol::error(const var& id, int code, const String& message, var data)
{
    auto e = object({{"code", code}, {"message", message}});
    if (!data.isVoid()) e.getDynamicObject()->setProperty("data", data);
    auto validId = id.isString() || id.isInt() || id.isInt64() ? id : var();
    return object({{"jsonrpc", "2.0"}, {"id", validId}, {"error", e}});
}
var Protocol::capabilities() const
{
    return object({{"tools", object()}, {"resources", object({{"subscribe", true}})}, {"prompts", object()}});
}
HttpReply Protocol::dispatch(const HttpRequest& request, Context& context)
{
    HttpReply reply; var id;
    try
    {
        if (request.host != "127.0.0.1:" + String(port) && request.host != "localhost:" + String(port))
            throw Error(-32003, "Host is not permitted");
        if (request.origin.isNotEmpty() && request.origin != "http://127.0.0.1:" + String(port)
            && request.origin != "http://localhost:" + String(port)) throw Error(-32003, "Origin is not permitted");
        if (!request.authorization.startsWith("Bearer ")) throw Error(-32001, "Authentication required");
        context.grant = permissions.authenticate(request.authorization.substring(7));
        if (request.method != "POST" && request.method != "DELETE" && request.method != "GET") { reply.status = 405; return reply; }
        if (request.body.getNumBytesAsUTF8() > 1024 * 1024) { reply.status = 413; return reply; }
        var message;
        if (request.method == "POST")
        {
            if (request.contentType.upToFirstOccurrenceOf(";", false, false).trim() != "application/json") { reply.status = 415; return reply; }
            if (!request.accept.contains("application/json") || !request.accept.contains("text/event-stream")) { reply.status = 406; return reply; }
            message = parseBounded(request.body);
            id = message["id"];
            if (message.getDynamicObject() == nullptr || message["jsonrpc"].toString() != "2.0" || !message["method"].isString()
                || (message.hasProperty("id") && !id.isString() && !id.isInt() && !id.isInt64())
                || (message.hasProperty("params") && message["params"].getDynamicObject() == nullptr)) throw Error(-32600, "Invalid request");
        }
        auto method = message["method"].toString(); auto params = message.getProperty("params", object());
        auto version = request.version;
        if (version.isNotEmpty() && !supported(version))
            throw Error(-32022, "Unsupported protocol version", object({{"supported", array({"2026-07-28", "2025-11-25", "2025-06-18"})}, {"requested", version}}));
        const bool modern = version == "2026-07-28" || params["_meta"].hasProperty("io.modelcontextprotocol/protocolVersion");
        if (modern)
        {
            if (request.method != "POST") { reply.status = 405; return reply; }
            if (!supported(version)) throw Error(-32022, "Unsupported protocol version", object({{"supported", array({"2026-07-28", "2025-11-25", "2025-06-18"})}, {"requested", version}}));
            if (version != "2026-07-28" || params["_meta"]["io.modelcontextprotocol/protocolVersion"].toString() != version
                || request.rpcMethod != method || params["_meta"]["io.modelcontextprotocol/clientInfo"].getDynamicObject() == nullptr
                || params["_meta"]["io.modelcontextprotocol/clientCapabilities"].getDynamicObject() == nullptr) throw Error(-32020, "Request metadata headers mismatch");
            if (method == "tools/call" || method == "resources/read" || method == "prompts/get")
            {
                auto name = params[method == "resources/read" ? "uri" : "name"].toString();
                auto headerName = request.rpcName;
                if (headerName.startsWith("=?base64?") && headerName.endsWith("?="))
                {
                    juce::MemoryOutputStream decoded;
                    if (!juce::Base64::convertFromBase64(decoded, headerName.substring(9, headerName.length() - 2)))
                        throw Error(-32020, "Malformed request name header");
                    headerName = decoded.toUTF8();
                }
                if (headerName != name) throw Error(-32020, "Request name header mismatch");
            }
        }
        else if (method == "initialize")
        {
            version = params["protocolVersion"].toString();
            if (version != "2025-11-25" && version != "2025-06-18") version = "2025-11-25";
            auto now = juce::Time::getMillisecondCounterHiRes();
            for (auto it = sessions.begin(); it != sessions.end();) if (now - it->second.touched > 3600000) it = sessions.erase(it); else ++it;
            if (sessions.size() >= 128) { reply.status = 429; return reply; }
            reply.session = secureToken();
            sessions[reply.session] = {context.grant.digest, version, false, now};
            reply.body = object({{"jsonrpc", "2.0"}, {"id", id}, {"result", object({{"protocolVersion", version},
                {"serverInfo", object({{"name", appName}, {"version", appVersion}})}, {"capabilities", capabilities()}})}});
            return reply;
        }
        else
        {
            auto it = sessions.find(request.session);
            if (it == sessions.end() || it->second.credential != context.grant.digest) { reply.status = 404; return reply; }
            if (request.method == "DELETE") { sessions.erase(it); reply.status = 204; return reply; }
            if (request.method == "GET")
            {
                if (!it->second.initialized) throw Error(-32600, "Session is not initialized");
                juce::Array<var> uris; for (const auto& uri : it->second.subscriptions) uris.add(uri);
                reply.stream = true; reply.notifications = object({{"resourceSubscriptions", uris}}); return reply;
            }
            if (version != it->second.version) throw Error(-32600, "Session protocol version mismatch");
            it->second.touched = juce::Time::getMillisecondCounterHiRes();
            if (method == "notifications/initialized" && !message.hasProperty("id"))
            { it->second.initialized = true; reply.status = 202; return reply; }
            if (!it->second.initialized && method != "ping") throw Error(-32600, "Session is not initialized");
        }
        context.check();
        if (!message.hasProperty("id")) { reply.status = 202; return reply; }
        var result;
        if (method == "server/discover" && modern)
            result = object({{"_meta", object({{"io.modelcontextprotocol/serverInfo", object({{"name", appName}, {"version", appVersion}})}})},
                {"capabilities", capabilities()}, {"supportedVersions", array({"2026-07-28", "2025-11-25", "2025-06-18"})}});
        else if (method == "ping") result = object();
        else if (method == "tools/list") result = registry.tools(params["cursor"].toString());
        else if (method == "resources/list") result = registry.resources(params["cursor"].toString());
        else if (method == "resources/templates/list") result = object({{"resourceTemplates", array({})}});
        else if (method == "prompts/list") result = registry.prompts(params["cursor"].toString());
        else if (method == "resources/read") result = registry.read(params["uri"].toString(), context);
        else if ((method == "resources/subscribe" || method == "resources/unsubscribe") && !modern)
        {
            auto uri = params["uri"].toString(); registry.read(uri, context);
            auto& uris = sessions.at(request.session).subscriptions;
            if (method == "resources/subscribe")
            {
                if (uris.size() >= 32) throw Error(-32602, "Subscription limit exceeded");
                uris.addIfNotAlreadyThere(uri);
            }
            else uris.removeString(uri);
            result = object();
        }
        else if (method == "subscriptions/listen" && modern)
        {
            auto filters = params["notifications"];
            if (filters.getDynamicObject() == nullptr) throw Error(-32602, "Missing notification filters");
            auto uris = filters["resourceSubscriptions"];
            if ((!uris.isVoid() && !uris.isArray()) || uris.size() > 32) throw Error(-32602, "Invalid resource subscriptions");
            if (uris.isArray()) for (const auto& uri : *uris.getArray())
            {
                if (!uri.isString()) throw Error(-32602, "Invalid resource URI");
                registry.read(uri.toString(), context);
            }
            reply.stream = true; reply.modern = true; reply.requestId = id;
            reply.notifications = object({{"resourceSubscriptions", uris.isVoid() ? array({}) : uris}});
            return reply;
        }
        else if (method == "prompts/get") result = registry.prompt(params["name"].toString());
        else if (method == "tools/call")
        {
            try
            {
                auto data = registry.call(params["name"].toString(), params.getProperty("arguments", object()), context);
                result = object({{"content", array({object({{"type", "text"}, {"text", juce::JSON::toString(data)}})})}, {"structuredContent", data}, {"isError", false}});
            }
            catch (const Error& e)
            {
                if (e.code == -32602) throw;
                result = object({{"content", array({object({{"type", "text"}, {"text", String(e.what())}})})}, {"isError", true}});
            }
        }
        else { reply.status = modern ? 404 : 200; reply.body = error(id, -32601, "Method not found"); return reply; }
        if (modern)
        {
            result.getDynamicObject()->setProperty("resultType", "complete");
            if (method.endsWith("/list") || method == "server/discover" || method == "resources/read")
            {
                result.getDynamicObject()->setProperty("ttlMs", 0);
                result.getDynamicObject()->setProperty("cacheScope", "private");
            }
        }
        reply.body = object({{"jsonrpc", "2.0"}, {"id", id}, {"result", result}});
    }
    catch (const Error& e)
    {
        reply.status = e.code == -32001 ? 401 : e.code == -32003 ? 403 : 400;
        reply.body = error(id, e.code, e.what(), e.data);
    }
    catch (const std::exception&) { reply.status = 500; reply.body = error(id, -32603, "Internal error"); }
    return reply;
}
}
