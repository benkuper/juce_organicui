#pragma once
#include "../mcp/McpCore.h"
#include <vector>

namespace organic::assistant
{
using mcp::String;
using mcp::var;
// These helpers are independent of the application and network for failure tests.
String base64url(const void*, size_t);
juce::MemoryBlock decode64(const String&);
String pkceChallenge(const String& verifier);
var verifyIdentity(const String& jwt, const var& jwks, const String& client,
                   const String& nonce, double nowSeconds);
var renewedAccount(const var& previous, const var& tokenResponse, double nowSeconds);
String accountKey(const var& verifiedIdentity, const String& issuedClient);
String usageError(const String& code);
juce::StringArray reasoningEfforts(const String& model);
String actionableError(const String& message, int code = 0);
String activityForTool(const String& name, const var& arguments);
enum class MessageTone { normal, warning, error };
struct MessageSpan { String text; MessageTone tone = MessageTone::normal; };
// Semantic fragments are independent of JUCE graphics for rendering regressions.
std::vector<MessageSpan> messageSpans(const String& line);
String errorFragment(const String& message);

// Windows: DPAPI current-user encryption. Unix: atomically replaced owner-only files.
// Neither tokens nor history are written to a project or to logs.
class ProtectedStore
{
public:
    explicit ProtectedStore(juce::File directory);
    var read(const String& name) const;
    void write(const String& name, const var&) const;
    const juce::File directory;
private:
    juce::File file(const String& name) const;
};

class ResponseStream
{
public:
    void push(const void* bytes, size_t count, const std::function<void(const String&)>& textDelta);
    void finish() const;
    var output = mcp::array({});
    bool completed = false;
    std::function<void(const var&)> eventObserved;
private:
    std::string pending, eventData;
    size_t total = 0;
    bool emittedText = false;
    std::map<int, var> completedItems;
    void event(const std::function<void(const String&)>&);
};
}
