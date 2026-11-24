#pragma once
#include "McpCore.h"
#include <deque>

namespace organic::mcp
{
// The provider is owned by the application and used only on its message thread.
class OrganicProvider
{
public:
    OrganicProvider(Registry&, ControllableContainer& root);
    String handle(ControllableContainer*);
    String handle(Controllable*);
    ControllableContainer* container(const String&) const;
    Controllable* controllable(const String&) const;
    var describe(const String&, bool includeData = false);
    void invalidateDocument();
    void ensureReady() const;
    juce::int64 generation() const { return documentGeneration; }
    std::function<unsigned(Controllable*)> parameterCapabilities;
    std::function<unsigned(Controllable*)> executionCapabilities;
    std::function<unsigned(ControllableContainer*, const String&)> creationCapabilities;
    std::function<void(ControllableContainer*, const String&, const Context&)> validateCreation;
    std::function<bool(ControllableContainer*)> hasExecutableBehavior;
    std::function<void()> documentInvalidated;
    void validateParameter(Parameter*, const var&, const Context&) const;
    var transaction(const String& name, const std::function<var()>& action);
    Registry& registry;
private:
    juce::WeakReference<ControllableContainer> root;
    juce::int64 documentGeneration = 1;
    struct Entry
    {
        juce::WeakReference<ControllableContainer> container;
        juce::WeakReference<Controllable> controllable;
        String signature;
        juce::int64 revision = 1;
    };
    std::map<String, Entry> handles;
    struct Transaction { String title; unsigned capabilities; juce::StringArray paths; };
    std::map<String, Transaction> transactions;
    struct Observation { juce::StringArray handles; std::deque<var> samples; double lastTime = 0; };
    std::map<String, Observation> observations;
    String registerHandle(ControllableContainer*, Controllable*);
    void requireRevision(const var&);
    var normalizedParameterValue(Parameter*, const var&) const;
    var list(const var&, Context&);
    void registerOperations();
};
}
