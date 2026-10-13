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
    return "ChatGPT request failed (" + code.substring(0, 100).retainCharacters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") + "). Retry or sign in again.";
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
    if (type == "response.output_text.delta") delta(value["delta"].toString());
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
        // This subscription route can omit already-streamed items from the
        // terminal envelope. Retain output_item.done, including local calls.
        if (output.getArray()->isEmpty()) for (auto& item : completedItems) output.append(item.second);
    }
    else if (type == "response.failed") throw Error(-32030, usageError(value["response"]["error"]["code"].toString()));
    else if (type == "error") throw Error(-32030, usageError(value["code"].toString()));
    else if (type == "response.incomplete") throw Error(-32030, "ChatGPT response is incomplete. Review activity before retrying.");
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
    if (!completed) throw Error(-32030, "ChatGPT stream interrupted before completion. Review activity before retrying.");
}
}
