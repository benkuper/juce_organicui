#include "AssistantCore.h"
#if JUCE_WINDOWS
 #include <windows.h>
 #include <wincrypt.h>
 #pragma comment(lib, "crypt32.lib")
#else
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
#endif

namespace organic::assistant
{
using mcp::object;
using mcp::Error;
std::vector<MessageSpan> messageSpans(const String& line)
{
    std::vector<MessageSpan> spans; MessageTone tone = MessageTone::normal; bool code = false;
    auto append = [&](const String& text, MessageTone value)
    {
        if (text.isEmpty()) return;
        if (!spans.empty() && spans.back().tone == value) spans.back().text += text;
        else spans.push_back({text, value});
    };
    for (int i = 0; i < line.length();)
    {
        auto rest = line.substring(i);
        if (line[i] == '`') { code = !code; append("`", tone); ++i; continue; }
        if (!code)
        {
            if (rest.startsWith("<error>")) { tone = MessageTone::error; i += 7; continue; }
            if (rest.startsWith("<warning>")) { tone = MessageTone::warning; i += 9; continue; }
            if (rest.startsWith("</error>")) { tone = MessageTone::normal; i += 8; continue; }
            if (rest.startsWith("</warning>")) { tone = MessageTone::normal; i += 10; continue; }
            bool boundary = i == 0 || juce::CharacterFunctions::isWhitespace(line[i - 1]);
            if (tone == MessageTone::normal && boundary)
            {
                bool error = rest.startsWithIgnoreCase("Error:") || rest.startsWithIgnoreCase("Failed:");
                bool warning = rest.startsWithIgnoreCase("Attention:") || rest.startsWithIgnoreCase("Warning:");
                if (error || warning)
                {
                    int end = i;
                    while (end < line.length() && line[end] != '.' && line[end] != '!' && line[end] != '?' && line[end] != '\n') ++end;
                    if (end < line.length() && line[end] != '\n') ++end;
                    append(line.substring(i, end), error ? MessageTone::error : MessageTone::warning); i = end; continue;
                }
                String label;
                for (auto value : {"What to try:", "Next step:", "Needs your input:", "Remaining:"})
                    if (rest.startsWithIgnoreCase(value)) { label = rest.substring(0, String(value).length()); break; }
                if (label.isNotEmpty()) { append(label, MessageTone::warning); i += label.length(); continue; }
            }
        }
        append(line.substring(i, i + 1), tone); ++i;
    }
    return spans;
}
String errorFragment(const String& message)
{
    auto end = message.indexOf("\n\n");
    if (end < 0) end = message.length();
    return "<error>" + message.substring(0, end) + "</error>" + message.substring(end);
}
String activityForTool(const String& name, const var& args)
{
    if (name == "organic_batch")
    {
        String first;
        if (auto* operations = args["operations"].getArray()) for (auto operation : *operations)
        {
            auto child = operation["name"].toString();
            if (child == "organic_batch") continue;
            auto label = activityForTool(child, operation["arguments"]);
            if (first.isEmpty()) first = label;
            if (child == "organic_create" || child == "organic_add_controllable" || child.contains("command")) return label;
        }
        return first.isEmpty() ? String("Applying grouped changes...") : first;
    }
    if (name == "assistant_task") return "Checking requested work...";
    if (name == "organic_create")
    {
        auto type = args["type"].toString().toLowerCase();
        if (type.contains("preset")) return "Creating presets...";
        if (type.contains("sequence")) return "Creating sequences...";
        if (type.contains("trigger")) return "Creating triggers...";
        if (type.contains("action") || type.contains("consequence")) return "Setting up actions...";
        if (type.contains("custom") && type.contains("var")) return "Configuring custom variables...";
        return "Creating project objects...";
    }
    if (name == "organic_browse" || name == "organic_search" || name == "organic_types") return "Exploring the project...";
    if (name == "organic_inspect") return "Inspecting project settings...";
    if (name == "organic_add_controllable" || name == "organic_set_range") return "Configuring values and ranges...";
    if (name == "organic_set_value" || name == "organic_rename" || name == "organic_move") return "Configuring project settings...";
    if (name == "organic_remove") return "Removing project objects...";
    if (name.contains("command")) return "Setting up actions...";
    if (name.startsWith("timeline_")) return "Setting up the sequence...";
    if (name.contains("diagnostic")) return "Checking project health...";
    if (name.contains("script")) return "Working on scripts...";
    if (name.contains("save")) return "Saving the project...";
    if (name == "chataigne_new") return "Starting a new project...";
    if (name == "chataigne_project") return "Inspecting the project...";
    return "Working on your project...";
}
String base64url(const void* data, size_t size)
{
    return juce::Base64::toBase64(data, size).replaceCharacter('+', '-').replaceCharacter('/', '_').removeCharacters("=");
}
juce::MemoryBlock decode64(const String& text)
{
    if (text.isEmpty() || text.length() > 131072 || text.containsAnyOf(" \r\n\t=+/")
        || text.containsOnly("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_") == false)
        throw Error(-32030, "Invalid base64url encoding");
    auto padded = text.replaceCharacter('-', '+').replaceCharacter('_', '/');
    while (padded.length() % 4) padded += "=";
    juce::MemoryOutputStream bytes;
    if (!juce::Base64::convertFromBase64(bytes, padded)) throw Error(-32030, "Invalid base64url encoding");
    return bytes.getMemoryBlock();
}
String pkceChallenge(const String& verifier)
{
    auto hash = juce::SHA256(verifier.toRawUTF8(), verifier.getNumBytesAsUTF8()).getRawData();
    return base64url(hash.getData(), hash.getSize());
}
static juce::BigInteger unsignedInteger(const juce::MemoryBlock& bytes)
{
    juce::BigInteger value;
    value.parseString(String::toHexString(bytes.getData(), (int) bytes.getSize(), 0), 16);
    return value;
}
var verifyIdentity(const String& jwt, const var& jwks, const String& client, const String& nonce, double now)
{
    auto parts = juce::StringArray::fromTokens(jwt, ".", "");
    if (parts.size() != 3 || client.isEmpty() || client == "dynamic_agent_client") throw Error(-32030, "Invalid identity token");
    auto headerBytes = decode64(parts[0]);
    auto header = mcp::parseBounded(String::fromUTF8((const char*) headerBytes.getData(), (int) headerBytes.getSize()));
    if (header["alg"].toString() != "RS256" || header["kid"].toString().isEmpty() || header.hasProperty("crit"))
        throw Error(-32030, "Unsupported identity signature");
    var selected;
    if (auto* keys = jwks["keys"].getArray())
        for (auto key : *keys) if (key["kid"].toString() == header["kid"].toString())
        {
            if (!selected.isVoid()) throw Error(-32030, "Ambiguous signing key");
            selected = key;
        }
    if (selected["kty"].toString() != "RSA" || (selected.hasProperty("use") && selected["use"].toString() != "sig")
        || (selected.hasProperty("alg") && selected["alg"].toString() != "RS256")) throw Error(-32030, "Unknown identity signing key");
    auto modulusBytes = decode64(selected["n"].toString());
    auto modulus = unsignedInteger(modulusBytes), exponent = unsignedInteger(decode64(selected["e"].toString()));
    auto signatureBytes = decode64(parts[2]);
    auto signature = unsignedInteger(signatureBytes);
    if (modulus.getHighestBit() < 2047 || modulus.getHighestBit() > 8191 || exponent < 3
        || exponent.getHighestBit() > 31 || !exponent[0] || signatureBytes.getSize() != modulusBytes.getSize()
        || signature >= modulus || signature.isZero()) throw Error(-32030, "Invalid RSA identity signature");
    signature.exponentModulo(exponent, modulus);
    const String signedText = parts[0] + "." + parts[1];
    auto hash = juce::SHA256(signedText.toRawUTF8(), signedText.getNumBytesAsUTF8()).getRawData();
    const unsigned char digestInfo[] = {0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20};
    juce::MemoryBlock encoded(modulusBytes.getSize(), true);
    auto* padding = static_cast<unsigned char*>(encoded.getData());
    padding[1] = 1;
    auto separator = encoded.getSize() - sizeof(digestInfo) - hash.getSize() - 1;
    std::fill(padding + 2, padding + separator, 0xff);
    std::memcpy(padding + separator + 1, digestInfo, sizeof(digestInfo));
    std::memcpy(padding + separator + 1 + sizeof(digestInfo), hash.getData(), hash.getSize());
    if (signature != unsignedInteger(encoded)) throw Error(-32030, "Identity signature verification failed");
    auto claimBytes = decode64(parts[1]);
    auto claims = mcp::parseBounded(String::fromUTF8((const char*) claimBytes.getData(), (int) claimBytes.getSize()));
    bool audience = claims["aud"].isString() && claims["aud"].toString() == client;
    if (auto* audiences = claims["aud"].getArray()) audience = audiences->contains(var(client)) && (audiences->size() == 1 || claims["azp"].toString() == client);
    if (claims.hasProperty("azp") && claims["azp"].toString() != client) audience = false;
    if (claims["iss"].toString() != "https://auth.openai.com") throw Error(-32030, "Identity token issuer verification failed");
    if (!audience) throw Error(-32030, "Identity token audience verification failed");
    if (claims["sub"].toString().isEmpty()) throw Error(-32030, "Identity token has no account subject");
    if ((double) claims["exp"] <= now) throw Error(-32030, "Identity token has expired");
    if ((double) claims["iat"] > now + 60 || (claims.hasProperty("nbf") && (double) claims["nbf"] > now + 60)) throw Error(-32030, "Identity token clock verification failed");
    if (!nonce.isEmpty() && claims["nonce"].toString() != nonce) throw Error(-32030, "Identity token nonce verification failed");
    return claims;
}
String accountKey(const var& identity, const String& client)
{
    return mcp::fingerprint(identity["iss"].toString() + "\n" + identity["sub"].toString() + "\n" + client);
}
var renewedAccount(const var& previous, const var& response, double now)
{
    if (response["access_token"].toString().isEmpty() || response["refresh_token"].toString().isEmpty()
        || !response["token_type"].toString().equalsIgnoreCase("Bearer") || (double) response["expires_in"] <= 0)
        throw Error(-32030, "Incomplete token response; sign in again");
    auto scopes = response.hasProperty("scope") ? response["scope"].toString() : previous["scope"].toString();
    if (!juce::StringArray::fromTokens(scopes, " ", "").contains("chatgpt.tokens.use.direct"))
        throw Error(-32030, "ChatGPT plan use is not enabled. Enable app plan access in ChatGPT and sign in again.");
    auto next = previous.clone();
    auto* record = next.getDynamicObject();
    if (!record) throw Error(-32030, "Invalid account record");
    record->setProperty("access_token", response["access_token"]);
    record->setProperty("refresh_token", response["refresh_token"]);
    if (response.hasProperty("id_token")) record->setProperty("id_token", response["id_token"]);
    record->setProperty("scope", scopes);
    record->setProperty("expires_at", now + (double) response["expires_in"]);
    record->setProperty("earliest_refresh_at", response["earliest_refresh_at"]);
    return next;
}
String usageError(const String& code)
{
    if (code == "subscription_sharing_usage_limit_exceeded") return "ChatGPT app usage limit reached. Review ChatGPT Settings > Usage or wait for your limit to reset.";
    if (code == "subscription_sharing_usage_unavailable") return "ChatGPT plan usage is unavailable for this account. Review app access in ChatGPT Settings > Usage.";
    if (code == "context_length_exceeded") return "This conversation is full for the selected model. Start a new conversation with the saved progress summary.";
    return "ChatGPT couldn't complete the request" + (code.isEmpty() ? String(".") : " (" + code + ").");
}
juce::StringArray reasoningEfforts(const String& model)
{
    // Keep unknown/chat/pro variants on the server default rather than send
    // an effort they may not accept. These families document this common set.
    if (model.contains("chat") || model.endsWith("-pro") || model.contains("-pro-")) return {};
    if (model.startsWith("gpt-6") || model.startsWith("gpt-5.6")) return {"low", "medium", "high", "xhigh", "max"};
    if (model.startsWith("gpt-5.5") || model.startsWith("gpt-5.4")) return {"low", "medium", "high", "xhigh"};
    if (model.startsWith("gpt-5") || model == "o3" || model == "o4-mini") return {"low", "medium", "high"};
    return {};
}
String actionableError(const String& message, int code)
{
    if (message.contains("What to try:")) return message;
    String next;
    auto lower = message.toLowerCase();
    if (code == -32800) return message;
    if (lower.contains("usage limit")) next = "Open Account and settings, then Usage to check your allowance and reset time. Choose Resume unfinished after the limit resets.";
    else if (lower.contains("plan usage")) next = "Open Account and settings, then Usage to check this account's app access and plan eligibility. Enable access if available, then sign in again.";
    else if (lower.contains("approved folders")) next = "Open Account and settings, add the needed folder under Approved folders, choose Remember access, then choose Resume unfinished.";
    else if (code == -32003) next = "Choose the access level needed for this task in the panel (Edit permits creating and configuring objects), then choose Resume unfinished. Script execution also needs Advanced execution in settings.";
    else if (code == -32001) next = "The assistant's local access was revoked or expired. Reconnect the selected provider in Account and settings, then review the access level.";
    else if (code == -32005 || code == -32006) next = "The project changed while the assistant was working. Choose Resume unfinished; the assistant will inspect the current objects and retry remaining changes without duplicating completed work.";
    else if (lower.contains("incompatible target") || lower.contains("target container")) next = "The selected object cannot be used for that connection. Choose Resume unfinished so the assistant can inspect the target setting and find a compatible object. If you intended a specific target, name it in a follow-up message.";
    else if (lower.contains("effort")) next = "Set Effort to Auto or choose a supported level for this model, then choose Resume unfinished.";
    else if (lower.contains("sign in") || lower.contains("authorize")) next = "Use the selected provider's sign-in action in Account and settings. Your saved conversations will still be available.";
    else if (lower.contains("network") || lower.contains("connect") || lower.contains("unavailable")) next = "Check your internet connection and use Reconnect in the panel (or Refresh models in settings), then retry. Your saved sign-in and conversations are kept.";
    else if (lower.contains("conversation is full") || lower.contains("conversation request is full")) next = "Start a new conversation from History and describe the work that remains.";
    else if (lower.contains("limit before verifying") || lower.contains("stopped making progress")) next = "Read the saved progress summary, then choose Resume unfinished. The assistant will inspect completed changes and continue the remaining checklist. If the same step keeps failing, copy the conversation to Codex with that step.";
    else next = "Choose Resume unfinished to inspect saved changes and retry the remaining work. If the same problem returns, use Copy conversation and paste it into Codex with the unfinished step from the progress summary.";
    return message + "\n\nWhat to try: " + next;
}
ProtectedStore::ProtectedStore(juce::File d) : directory(d)
{
    if (directory.createDirectory().failed()) throw Error(-32030, "Cannot create assistant credential directory");
#if !JUCE_WINDOWS
    struct stat info {};
    if (::lstat(directory.getFullPathName().toRawUTF8(), &info) != 0 || !S_ISDIR(info.st_mode)
        || info.st_uid != ::getuid() || ::chmod(directory.getFullPathName().toRawUTF8(), 0700) != 0)
        throw Error(-32030, "Assistant storage must be an owned directory");
#endif
}
juce::File ProtectedStore::file(const String& name) const
{
    if (name.isEmpty() || !name.containsOnly("abcdefghijklmnopqrstuvwxyz0123456789-_")) throw Error(-32030, "Invalid protected record name");
    return directory.getChildFile(name + ".private");
}
var ProtectedStore::read(const String& name) const
{
    auto path = file(name);
    if (!path.existsAsFile()) return {};
    juce::MemoryBlock bytes;
#if JUCE_WINDOWS
    if (!path.loadFileAsData(bytes) || bytes.getSize() > 4 * 1024 * 1024) throw Error(-32030, "Cannot read protected record");
    DATA_BLOB source { (DWORD) bytes.getSize(), (BYTE*) bytes.getData() }, plain {};
    if (!CryptUnprotectData(&source, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &plain)) throw Error(-32030, "Cannot unlock assistant storage for this user");
    auto json = String::fromUTF8((const char*) plain.pbData, (int) plain.cbData);
    SecureZeroMemory(plain.pbData, plain.cbData); LocalFree(plain.pbData);
#else
    int descriptor = ::open(path.getFullPathName().toRawUTF8(), O_RDONLY | O_NOFOLLOW);
    struct stat info {};
    if (descriptor < 0) throw Error(-32030, "Cannot open protected record");
    if (::fstat(descriptor, &info) || !S_ISREG(info.st_mode) || info.st_uid != ::getuid() || (info.st_mode & 077) || info.st_size > 1024 * 1024)
    { ::close(descriptor); throw Error(-32030, "Assistant record permissions must be owner-only"); }
    bytes.setSize((size_t) info.st_size); size_t offset = 0;
    while (offset < bytes.getSize()) { auto n = ::read(descriptor, (char*) bytes.getData() + offset, bytes.getSize() - offset); if (n <= 0) break; offset += (size_t) n; }
    ::close(descriptor);
    if (offset != bytes.getSize()) throw Error(-32030, "Cannot read protected record");
    auto json = String::fromUTF8((const char*) bytes.getData(), (int) bytes.getSize());
#endif
    return mcp::parseBounded(json);
}
void ProtectedStore::write(const String& name, const var& value) const
{
    auto json = juce::JSON::toString(value, true);
    if (json.getNumBytesAsUTF8() > 1024 * 1024) throw Error(-32030, "Assistant history is full. Delete history to continue.");
    auto path = file(name), temporary = directory.getChildFile(".pending-" + mcp::secureToken());
#if JUCE_WINDOWS
    DATA_BLOB source { (DWORD) json.getNumBytesAsUTF8(), (BYTE*) json.toRawUTF8() }, encrypted {};
    if (!CryptProtectData(&source, L"OrganicUI Assistant", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &encrypted)) throw Error(-32030, "Cannot protect assistant storage");
    auto ok = temporary.replaceWithData(encrypted.pbData, encrypted.cbData);
    LocalFree(encrypted.pbData);
    if (!ok || !MoveFileExW(temporary.getFullPathName().toWideCharPointer(), path.getFullPathName().toWideCharPointer(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    { temporary.deleteFile(); throw Error(-32030, "Cannot save protected record"); }
#else
    int descriptor = ::open(temporary.getFullPathName().toRawUTF8(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (descriptor < 0) throw Error(-32030, "Cannot create protected record");
    size_t offset = 0, length = json.getNumBytesAsUTF8();
    while (offset < length) { auto n = ::write(descriptor, json.toRawUTF8() + offset, length - offset); if (n <= 0) break; offset += (size_t) n; }
    auto synced = ::fsync(descriptor) == 0; ::close(descriptor);
    if (offset != length || !synced || ::rename(temporary.getFullPathName().toRawUTF8(), path.getFullPathName().toRawUTF8()))
    { temporary.deleteFile(); throw Error(-32030, "Cannot save protected record"); }
#endif
}
void ResponseStream::event(const std::function<void(const String&)>& delta)
{
    if (eventData.empty()) return;
    auto data = String::fromUTF8(eventData.data(), (int) eventData.size()); eventData.clear();
    if (data == "[DONE]") return;
    auto value = mcp::parseBounded(data);
    auto type = value["type"].toString();
    if (eventObserved) eventObserved(value);
    if (completed) throw Error(-32030, "Unexpected event after response completion");
    if (type == "response.output_text.delta") { emittedText = true; delta(value["delta"].toString()); }
    else if (type == "response.output_item.done")
    {
        if (!value["item"].getDynamicObject() || !value["output_index"].isInt() || (int) value["output_index"] < 0 || (int) value["output_index"] > 127)
            throw Error(-32030, "Invalid completed response item");
        completedItems[(int) value["output_index"]] = value["item"].clone();
    }
    else if (type == "response.completed")
    {
        if (!value["response"]["output"].isArray()) throw Error(-32030, "Invalid completed response");
        completed = true; output = value["response"]["output"].clone();
        // The terminal envelope can omit some or all already-streamed items.
        // Retain missing calls even when a final message is also present.
        for (auto& entry : completedItems)
        {
            auto item = entry.second; bool found = false;
            for (auto existing : *output.getArray())
                found |= (!item["id"].toString().isEmpty() && item["id"] == existing["id"])
                    || (!item["call_id"].toString().isEmpty() && item["call_id"] == existing["call_id"])
                    || juce::JSON::toString(item, true) == juce::JSON::toString(existing, true);
            if (!found) output.append(item);
        }
        if (!emittedText)
            for (auto item : *output.getArray()) if (item["type"].toString() == "message")
                if (auto* content = item["content"].getArray())
                    for (auto part : *content) if (part["type"].toString() == "output_text") delta(part["text"].toString());
    }
    else if (type == "response.failed" || type == "error")
    {
        auto error = value["response"]["error"];
        if (!error.isObject()) error = value["error"];
        if (!error.isObject()) error = value;
        auto code = error["code"].toString();
        auto detail = error["message"].toString().replace("\r", " ").replace("\n", " ").trim().substring(0, 1000);
        auto message = usageError(code);
        if (detail.isNotEmpty() && !code.startsWith("subscription_sharing_")) message += " " + detail;
        // An unclassified terminal failure may be transient. Retry the unexecuted
        // response twice, without replaying any completed project operation.
        throw Error(-32030, message, mcp::object({{"errorCode", code}, {"retryable", code.isEmpty() || code == "server_error" || code == "internal_error" || code == "rate_limit_exceeded" || code == "overloaded" || code == "service_unavailable"}}));
    }
    else if (type == "response.incomplete") throw Error(-32030, "ChatGPT response is incomplete. Review activity before retrying.", mcp::object({{"retryable", true}}));
}
void ResponseStream::push(const void* bytes, size_t count, const std::function<void(const String&)>& delta)
{
    total += count;
    if (total > 4 * 1024 * 1024) throw Error(-32030, "Response exceeds the assistant stream limit");
    pending.append((const char*) bytes, count);
    size_t end;
    while ((end = pending.find('\n')) != std::string::npos)
    {
        auto line = pending.substr(0, end); pending.erase(0, end + 1);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) event(delta);
        else if (line.compare(0, 5, "data:") == 0)
        { if (!eventData.empty()) eventData += '\n'; eventData += line.substr(line.size() > 5 && line[5] == ' ' ? 6 : 5); }
        if (eventData.size() + pending.size() > 1024 * 1024) throw Error(-32030, "Response event exceeds payload limit");
    }
}
void ResponseStream::finish() const
{
    if (!completed) throw Error(-32030, "ChatGPT stream interrupted before completion. Review activity before retrying.", mcp::object({{"retryable", true}}));
}
}
