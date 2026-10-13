#include "OrganicMcpProvider.h"
#include <cmath>

namespace organic::mcp
{
namespace
{
class ParameterRangeAction : public juce::UndoableAction
{
public:
    ParameterRangeAction(Parameter* p, var low, var high) : parameter(p), oldLow(p->minimumValue), oldHigh(p->maximumValue), low(low), high(high) {}
    bool perform() override { if (!parameter) return false; parameter->setRange(low, high); return true; }
    bool undo() override { if (!parameter) return false; parameter->setRange(oldLow, oldHigh); return true; }
private:
    juce::WeakReference<Parameter> parameter; var oldLow, oldHigh, low, high;
};
class EnumOptionsAction : public juce::UndoableAction
{
public:
    EnumOptionsAction(EnumParameter* p, const var& options) : parameter(p), previousValue(p->getValue())
    {
        for (auto* entry : p->enumValues) previous.add(*entry);
        for (const auto& entry : *options.getArray()) next.add({entry["key"].toString(), entry["value"]});
    }
    bool perform() override { auto* p = dynamic_cast<EnumParameter*>(parameter.get()); if (!p) return false; p->setOptions(next); return true; }
    bool undo() override { auto* p = dynamic_cast<EnumParameter*>(parameter.get()); if (!p) return false; p->setOptions(previous); p->setValue(previousValue); return true; }
private:
    juce::WeakReference<Parameter> parameter; juce::Array<EnumParameter::EnumValue> previous, next; var previousValue;
};
}
OrganicProvider::OrganicProvider(Registry& r, ControllableContainer& applicationRoot) : registry(r), root(&applicationRoot)
{
    // Applications may refine these classifications but cannot remove the base
    // file and script checks in validateParameter or the registry capability gate.
    parameterCapabilities = [](Controllable*) { return static_cast<unsigned>(edit); };
    executionCapabilities = [](Controllable*) { return static_cast<unsigned>(control); };
    creationCapabilities = [](ControllableContainer* parent, const String& type)
    { return static_cast<unsigned>(edit | ((dynamic_cast<ScriptManager*>(parent) != nullptr || type.containsIgnoreCase("script")) ? advanced : 0)); };
    hasExecutableBehavior = [](ControllableContainer* cc) { return dynamic_cast<Script*>(cc) != nullptr; };
    registerOperations();
}
void OrganicProvider::ensureReady() const
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    if (root == nullptr || root->isClearing || root->isCurrentlyLoadingData
        || (Engine::mainEngine != nullptr && (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing)))
        throw Error(-32005, "Project is loading or unavailable");
}
void OrganicProvider::invalidateDocument()
{
    ++documentGeneration; handles.clear(); transactions.clear(); observations.clear();
    if (documentInvalidated) documentInvalidated();
}
String OrganicProvider::registerHandle(ControllableContainer* cc, Controllable* c)
{
    ensureReady();
    for (const auto& entry : handles)
        if ((cc != nullptr && entry.second.container == cc) || (c != nullptr && entry.second.controllable == c)) return entry.first;
    if (handles.size() >= 50000)
    {
        for (auto it = handles.begin(); it != handles.end();) if (it->second.container == nullptr && it->second.controllable == nullptr) it = handles.erase(it); else ++it;
        if (handles.size() >= 50000) throw Error(-32000, "Object handle limit reached; use scoped queries");
    }
    auto id = String(documentGeneration) + ":" + juce::Uuid().toString();
    Entry entry; entry.container = cc; entry.controllable = c; handles.emplace(id, entry); return id;
}
String OrganicProvider::handle(ControllableContainer* cc) { return registerHandle(cc, nullptr); }
String OrganicProvider::handle(Controllable* c) { return registerHandle(nullptr, c); }
ControllableContainer* OrganicProvider::container(const String& id) const
{
    ensureReady(); auto it = handles.find(id);
    if (it == handles.end() || it->second.container == nullptr) throw Error(-32005, "Container handle is stale or incompatible");
    return it->second.container.get();
}
Controllable* OrganicProvider::controllable(const String& id) const
{
    ensureReady(); auto it = handles.find(id);
    if (it == handles.end() || it->second.controllable == nullptr) throw Error(-32005, "Parameter handle is stale or incompatible");
    return it->second.controllable.get();
}
static bool secretName(const String& name)
{
    auto n = name.toLowerCase();
    return n.contains("password") || n.contains("secret") || n.contains("token") || n.contains("apikey") || n.contains("authorization");
}
static var redact(var data, int depth = 0)
{
    if (depth > 24) return "[nesting limit]";
    if (data.getDynamicObject() != nullptr)
    {
        const bool sensitive = secretName(data["controlAddress"].toString()) || secretName(data["niceName"].toString()) || secretName(data["shortName"].toString());
        for (const auto& property : data.getDynamicObject()->getProperties())
        {
            auto replacement = secretName(property.name.toString()) || (sensitive && property.name.toString() == "value")
                ? var("[redacted]") : redact(property.value, depth + 1);
            data.getDynamicObject()->setProperty(property.name, replacement);
        }
    }
    else if (data.isArray()) for (auto& value : *data.getArray()) value = redact(value, depth + 1);
    return data;
}
var OrganicProvider::describe(const String& id, bool includeData)
{
    ensureReady(); auto it = handles.find(id);
    if (it == handles.end()) throw Error(-32005, "Object handle is stale");
    auto& entry = it->second;
    var result;
    if (auto* c = entry.controllable.get())
    {
        auto* parameter = dynamic_cast<Parameter*>(c);
        result = object({{"handle", id}, {"kind", parameter ? "parameter" : "trigger"}, {"name", c->niceName},
            {"address", c->getControlAddress()}, {"type", c->getTypeString()}, {"description", c->description},
            {"enabled", c->enabled}, {"feedbackOnly", c->isControllableFeedbackOnly}, {"generation", documentGeneration}});
        if (parameter)
        {
            auto value = parameter->getValue();
            result.getDynamicObject()->setProperty("value", secretName(c->shortName) ? var("[redacted]") : value);
            result.getDynamicObject()->setProperty("minimum", parameter->minimumValue);
            result.getDynamicObject()->setProperty("maximum", parameter->maximumValue);
            result.getDynamicObject()->setProperty("requiredCapabilities", (int) parameterCapabilities(c));
            if (auto* choices = dynamic_cast<EnumParameter*>(c))
            { juce::Array<var> keys; for (const auto& key : choices->getAllKeys()) keys.add(key); result.getDynamicObject()->setProperty("choices", keys); }
        }
    }
    else if (auto* cc = entry.container.get())
    {
        result = object({{"handle", id}, {"kind", "container"}, {"name", cc->niceName}, {"address", cc->getControlAddress()},
            {"generation", documentGeneration}, {"childCount", cc->controllableContainers.size()}, {"parameterCount", cc->controllables.size()}});
        if (auto* item = dynamic_cast<BaseItem*>(cc)) result.getDynamicObject()->setProperty("type", item->getTypeString());
        juce::Array<var> controllableTypes;
        if (cc->userCanAddControllables && !cc->customUserCreateControllableFunc)
            for (auto* definition : ControllableFactory::getInstance()->controllableDefs)
                if (cc->userAddControllablesFilters.isEmpty() || cc->userAddControllablesFilters.contains(definition->controllableType))
                    controllableTypes.add(definition->controllableType);
        result.getDynamicObject()->setProperty("controllableTypes", controllableTypes);
        if (auto* manager = dynamic_cast<ManagerAdapter*>(cc))
        {
            auto types = manager->mcpTypes(); juce::Array<var> page;
            for (int index = 0; index < juce::jmin(100, types.size()); ++index) page.add(types[index]);
            result.getDynamicObject()->setProperty("types", page); result.getDynamicObject()->setProperty("typeCount", types.size());
        }
    }
    else throw Error(-32005, "Object was deleted");
    result.getDynamicObject()->setProperty("resourceUri", "organic://object/" + juce::URL::addEscapeChars(id, true));
    auto revisionState = result.clone();
    if (auto* p = dynamic_cast<Parameter*>(entry.controllable.get())) revisionState.getDynamicObject()->setProperty("unredactedValue", p->getValue());
    if (auto* cc = entry.container.get())
    {
        juce::Array<var> children, parameters;
        for (auto weak : cc->controllableContainers) if (weak) children.add(weak->getControlAddress());
        for (auto* p : cc->controllables)
        {
            if (auto* parameter = dynamic_cast<Parameter*>(p)) parameters.add(object({{"address", p->getControlAddress()}, {"value", parameter->getValue()}}));
        }
        revisionState.getDynamicObject()->setProperty("childOrder", children);
        revisionState.getDynamicObject()->setProperty("parameters", parameters);
    }
    auto signature = fingerprint(juce::JSON::toString(revisionState, true));
    if (entry.signature.isNotEmpty() && entry.signature != signature) ++entry.revision;
    entry.signature = signature;
    result.getDynamicObject()->setProperty("revision", entry.revision);
    if (includeData)
    {
        auto data = entry.container != nullptr ? entry.container->getJSONData() : entry.controllable->getJSONData();
        if (juce::JSON::toString(data, true).getNumBytesAsUTF8() > 1024 * 1024) throw Error(-32000, "Serialized object exceeds the response limit; use scoped browsing");
        result.getDynamicObject()->setProperty("data", redact(data.clone()));
    }
    return result;
}
void OrganicProvider::requireRevision(const var& args)
{
    auto description = describe(args["handle"].toString());
    if ((juce::int64) args["revision"] != (juce::int64) description["revision"])
        throw Error(-32006, "Object changed since it was inspected", object({{"current", description}}));
}
var OrganicProvider::list(const var& args, Context& context)
{
    ensureReady();
    auto* scope = args["scope"].toString().isEmpty() ? root.get() : container(args["scope"].toString());
    auto query = args["query"].toString(); int offset = (int) args.getProperty("offset", 0), limit = (int) args.getProperty("limit", 100);
    juce::Array<var> rows; int index = 0;
    // Intentionally scoped to direct children; clients traverse pages explicitly.
    auto add = [&](const String& name, const std::function<String()>& getHandle)
    {
        context.check(); if (query.isNotEmpty() && !name.containsIgnoreCase(query)) return;
        if (index++ >= offset && rows.size() < limit) rows.add(describe(getHandle()));
    };
    for (auto* c : scope->controllables) add(c->niceName, [&] { return handle(c); });
    for (auto weak : scope->controllableContainers) if (auto* cc = weak.get()) add(cc->niceName, [&] { return handle(cc); });
    auto result = object({{"scope", handle(scope)}, {"items", rows}, {"generation", documentGeneration}});
    if (offset + rows.size() < index) result.getDynamicObject()->setProperty("nextOffset", offset + rows.size());
    return result;
}
void OrganicProvider::validateParameter(Parameter* p, const var& value, const Context& context) const
{
    if (p == nullptr || p->isControllableFeedbackOnly || !p->enabled) throw Error(-32602, "Parameter is not writable");
    context.require(parameterCapabilities(p));
    auto type = p->type;
    if ((type == Controllable::BOOL && !value.isBool())
        || ((type == Controllable::STRING || type == Controllable::ENUM || type == Controllable::TARGET) && !value.isString())
        || ((type == Controllable::INT || type == Controllable::FLOAT) && !(value.isInt() || value.isInt64() || value.isDouble()))
        || (type == Controllable::INT && std::floor((double) value) != (double) value)) throw Error(-32602, "Parameter value has the wrong type");
    if (type == Controllable::INT || type == Controllable::FLOAT)
        validate(object({{"type", "number"}, {"minimum", p->minimumValue}, {"maximum", p->maximumValue}}), value);
    if (p->isComplex())
    {
        int expected = type == Controllable::POINT2D ? 2 : type == Controllable::POINT3D ? 3 : type == Controllable::COLOR ? 4 : 0;
        if (expected == 0 || !value.isArray() || value.size() != expected) throw Error(-32602, "Invalid compound parameter value");
        for (const auto& v : *value.getArray()) validate(object({{"type", "number"}}), v);
    }
    if (auto* choices = dynamic_cast<EnumParameter*>(p)) if (choices->getEntryForKey(value.toString()) == nullptr) throw Error(-32602, "Unknown enumeration option");
    if (auto* file = dynamic_cast<FileParameter*>(p))
    {
        auto path = value.toString();
        if (path.isNotEmpty())
        {
            auto base = file->getBasePath();
            context.requireFile(juce::File::isAbsolutePath(path) ? juce::File(path) : base.getChildFile(path));
        }
    }
    if (auto* target = dynamic_cast<TargetParameter*>(p))
    {
        auto path = value.toString();
        if (path.isEmpty()) return;
        auto* scope = target->rootContainer.get(); if (scope == nullptr) scope = root.get();
        if (target->targetType == TargetParameter::CONTAINER)
        {
            auto* cc = scope->getControllableContainerForAddress(path, true);
            if (cc == nullptr || (target->defaultContainerTypeCheckFunc && !target->defaultContainerTypeCheckFunc(cc))) throw Error(-32602, "Incompatible target container");
        }
        else
        {
            auto* c = scope->getControllableForAddress(path);
            if (c == nullptr || (target->customTargetFilterFunc && !target->customTargetFilterFunc(c))
                || (!target->typesFilter.isEmpty() && !target->typesFilter.contains(c->getTypeString()))
                || target->excludeTypesFilter.contains(c->getTypeString())) throw Error(-32602, "Incompatible target parameter");
        }
    }
    for (auto* cc = p->parentContainer.get(); cc != nullptr; cc = cc->parentContainer.get())
        if (dynamic_cast<Script*>(cc) != nullptr) context.require(edit | advanced);
}
var OrganicProvider::transaction(const String& name, const std::function<var()>& action)
{
    auto id = juce::Uuid().toString(), title = "MCP " + id + ": " + name;
    auto* undo = UndoMaster::getInstance(); undo->beginGroupedTransaction(title);
    var result;
    try { result = action(); } catch (...) { undo->endGroupedTransaction(); throw; }
    undo->endGroupedTransaction();
    if (undo->getUndoDescription() == title)
    {
        auto* context = Registry::executingContext();
        transactions[id] = {title, context ? context->requiredCapabilities : static_cast<unsigned>(edit), context ? context->requiredPaths : juce::StringArray()};
        result.getDynamicObject()->setProperty("transaction", id);
    }
    return result;
}
void OrganicProvider::registerOperations()
{
    auto add = [this](String name, String description, var input, unsigned caps,
        std::function<var(const var&, Context&)> execute, std::function<void(const var&, const Context&)> prevalidate = {}, bool reversible = false, bool external = false)
    {
        Operation op; op.name = name; op.description = description; op.input = input; op.output = openObjectSchema();
        op.capabilities = caps; op.execute = execute; op.prevalidate = prevalidate; op.reversible = reversible; op.externalEffects = external; registry.add(op);
    };
    auto handleInput = schema(object({{"handle", stringSchema()}}), array({"handle"}));
    auto editInput = schema(object({{"handle", stringSchema()}, {"revision", integerSchema(1, INT32_MAX)}}), array({"handle", "revision"}));
    add("organic_browse", "Browse or search direct children in a scope. Start without scope; follow handles and nextOffset.",
        schema(object({{"scope", stringSchema()}, {"query", stringSchema()}, {"offset", integerSchema(0, INT32_MAX)}, {"limit", integerSchema(1, 200)}})), inspect,
        [this](const var& args, Context& c) { return list(args, c); });
    auto inspectInput = handleInput.clone(); inspectInput["properties"].getDynamicObject()->setProperty("includeData", object({{"type", "boolean"}}));
    add("organic_inspect", "Inspect a handle, its revision, parameter constraints, and available manager types. Optional includeData adds bounded serialized data.", inspectInput, inspect,
        [this](const var& args, Context&) { return describe(args["handle"].toString(), (bool) args["includeData"]); });
    add("organic_types", "Discover supported factory types on a manager, with pagination.",
        schema(object({{"handle", stringSchema()}, {"offset", integerSchema(0, INT32_MAX)}, {"limit", integerSchema(1, 200)}}), array({"handle"})), inspect,
        [this](const var& args, Context&) { auto* manager = dynamic_cast<ManagerAdapter*>(container(args["handle"].toString())); if (!manager) throw Error(-32602, "Handle is not a manager"); auto types = manager->mcpTypes(); int offset = (int) args.getProperty("offset", 0), limit = (int) args.getProperty("limit", 100); juce::Array<var> rows; for (int i = offset; i < types.size() && rows.size() < limit; ++i) rows.add(types[i]); auto result = object({{"types", rows}, {"total", types.size()}}); if (offset + rows.size() < types.size()) result.getDynamicObject()->setProperty("nextOffset", offset + rows.size()); return result; });
    auto setInput = editInput.clone(); setInput["properties"].getDynamicObject()->setProperty("value", anySchema()); setInput["required"].getArray()->add("value");
    auto validateSet = [this](const var& args, const Context& c)
    { requireRevision(args); validateParameter(dynamic_cast<Parameter*>(controllable(args["handle"].toString())), args["value"], c); };
    add("organic_set_value", "Set a typed parameter after inspecting its revision. Control permits existing runtime values; configuration requires Edit. Sensitive and file parameters require their grants.", setInput, control,
        [this](const var& args, Context& c) { return transaction("Set parameter", [&] { auto* p = dynamic_cast<Parameter*>(controllable(args["handle"].toString())); if (auto* file = dynamic_cast<FileParameter*>(p)) if (file->getValue().toString().isNotEmpty()) c.requiredPaths.addIfNotAlreadyThere(file->getFile().getFullPathName()); p->setUndoableValue(p->getValue(), args["value"]); return describe(args["handle"].toString()); }); }, validateSet, true);
    add("organic_trigger", "Operate an existing trigger. Authored behavior may emit output or invoke code; effects are irreversible.", handleInput, control,
        [this](const var& args, Context&) { auto* t = dynamic_cast<Trigger*>(controllable(args["handle"].toString())); t->trigger(); return object({{"executed", true}, {"reversible", false}}); },
        [this](const var& args, const Context& c) { auto* t = dynamic_cast<Trigger*>(controllable(args["handle"].toString())); if (t == nullptr || !t->enabled || t->isControllableFeedbackOnly) throw Error(-32602, "Target is not an operable trigger"); c.require(executionCapabilities(t)); for (auto* cc = t->parentContainer.get(); cc != nullptr; cc = cc->parentContainer.get()) if (dynamic_cast<Script*>(cc)) c.require(advanced); }, false, true);
    auto rangeInput = editInput.clone();
    for (const auto* property : {"minimum", "maximum"}) { rangeInput["properties"].getDynamicObject()->setProperty(property, object({{"type", "number"}})); rangeInput["required"].getArray()->add(property); }
    add("organic_set_range", "Set a numeric parameter range in one undo transaction.", rangeInput, edit,
        [this](const var& args, Context&) { return transaction("Set parameter range", [&] { auto* p = dynamic_cast<Parameter*>(controllable(args["handle"].toString())); UndoMaster::getInstance()->performAction("Set range", new ParameterRangeAction(p, args["minimum"], args["maximum"])); return describe(args["handle"].toString()); }); },
        [this](const var& args, const Context& c) { requireRevision(args); auto* p = dynamic_cast<Parameter*>(controllable(args["handle"].toString())); if (!p || !p->canHaveRange || (p->type != Controllable::INT && p->type != Controllable::FLOAT) || (double) args["minimum"] > (double) args["maximum"]) throw Error(-32602, "Invalid numeric parameter range"); c.require(parameterCapabilities(p)); if (p->type == Controllable::INT && (std::floor((double) args["minimum"]) != (double) args["minimum"] || std::floor((double) args["maximum"]) != (double) args["maximum"])) throw Error(-32602, "Integer ranges require integer bounds"); }, true);
    auto optionsInput = editInput.clone();
    optionsInput["properties"].getDynamicObject()->setProperty("options", object({{"type", "array"}, {"items", schema(object({{"key", stringSchema()}, {"value", anySchema()}}), array({"key", "value"}))}, {"minItems", 1}, {"maxItems", 64}}));
    optionsInput["required"].getArray()->add("options");
    add("organic_set_enum_options", "Replace an enumeration's options in one undo transaction.", optionsInput, edit,
        [this](const var& args, Context&) { return transaction("Set enumeration options", [&] { auto* p = dynamic_cast<EnumParameter*>(controllable(args["handle"].toString())); UndoMaster::getInstance()->performAction("Set enum options", new EnumOptionsAction(p, args["options"])); return describe(args["handle"].toString()); }); },
        [this](const var& args, const Context& c) { requireRevision(args); auto* p = dynamic_cast<EnumParameter*>(controllable(args["handle"].toString())); if (!p || p->isControllableFeedbackOnly) throw Error(-32602, "Target is not a writable enumeration"); c.require(parameterCapabilities(p)); juce::StringArray keys; for (const auto& option : *args["options"].getArray()) { auto key = option["key"].toString(); if (key.isEmpty() || keys.contains(key)) throw Error(-32602, "Enumeration keys must be nonempty and unique"); keys.add(key); } }, true);
    auto renameInput = editInput.clone(); renameInput["properties"].getDynamicObject()->setProperty("name", stringSchema()); renameInput["required"].getArray()->add("name");
    add("organic_rename", "Rename an object in one undo transaction.", renameInput, edit,
        [this](const var& args, Context&) { return transaction("Rename object", [&] { auto it = handles.find(args["handle"].toString()); if (it->second.container) it->second.container->setUndoableNiceName(args["name"].toString()); else it->second.controllable->setUndoableNiceName(args["name"].toString()); return describe(args["handle"].toString()); }); },
        [this](const var& args, const Context&) { requireRevision(args); if (args["name"].toString().trim().isEmpty()) throw Error(-32602, "Name must not be empty"); auto& e = handles.at(args["handle"].toString()); if ((e.container && !e.container->nameCanBeChangedByUser) || (e.controllable && !e.controllable->userCanChangeName)) throw Error(-32602, "Object cannot be renamed"); }, true);
    auto createInput = schema(object({{"handle", stringSchema()}, {"revision", integerSchema(1, INT32_MAX)}, {"type", stringSchema()}, {"name", stringSchema()}}), array({"handle", "revision", "type"}));
    add("organic_add_controllable", "Add a native parameter or trigger to a container that permits user controls. Discover controllableTypes first, then configure the returned handle.", createInput, edit,
        [this](const var& args, Context&) { return transaction("Add controllable", [&]
        {
            auto* parent = container(args["handle"].toString());
            auto control = std::unique_ptr<Controllable>(ControllableFactory::createControllable(args["type"].toString()));
            if (!control) throw Error(-32602, "Unsupported controllable type");
            control->setNiceName(parent->getUniqueNameInContainer(args["name"].toString().isEmpty() ? args["type"].toString() : args["name"].toString()));
            control->isRemovableByUser = true; control->userCanChangeName = true;
            auto* added = control.release(); parent->addUndoableControllable(added);
            return describe(handle(added));
        }); },
        [this](const var& args, const Context& c)
        {
            requireRevision(args); auto* parent = container(args["handle"].toString()); auto type = args["type"].toString();
            if (!parent->userCanAddControllables || parent->customUserCreateControllableFunc
                || (!parent->userAddControllablesFilters.isEmpty() && !parent->userAddControllablesFilters.contains(type))
                || !ControllableFactory::getTypesWithout({}).contains(type)) throw Error(-32602, "Container does not support this native controllable type");
            c.require(creationCapabilities(parent, type));
            for (auto* ancestor = parent; ancestor; ancestor = ancestor->parentContainer.get()) if (hasExecutableBehavior(ancestor)) c.require(advanced);
            if (validateCreation) validateCreation(parent, type, c);
        }, true);
    add("organic_remove_controllable", "Remove a user-removable parameter or trigger in one undo transaction.", editInput, edit,
        [this](const var& args, Context& c) { return transaction("Remove controllable", [&]
        {
            auto* control = controllable(args["handle"].toString());
            if (auto* file = dynamic_cast<FileParameter*>(control)) if (file->getValue().toString().isNotEmpty()) c.requiredPaths.addIfNotAlreadyThere(file->getFile().getFullPathName());
            control->parentContainer->removeUndoableControllable(control); return object({{"removed", true}});
        }); },
        [this](const var& args, const Context& c)
        {
            requireRevision(args); auto* control = controllable(args["handle"].toString());
            if (!control->isRemovableByUser || !control->parentContainer) throw Error(-32602, "Controllable cannot be removed");
            c.require(parameterCapabilities(control));
            for (auto* ancestor = control->parentContainer.get(); ancestor; ancestor = ancestor->parentContainer.get()) if (hasExecutableBehavior(ancestor)) c.require(advanced);
        }, true);
    add("organic_create", "Create a supported manager item by its discovered type; configure its parameters separately.", createInput, edit,
        [this](const var& args, Context&) { return transaction("Create object", [&] { auto* manager = dynamic_cast<ManagerAdapter*>(container(args["handle"].toString())); auto data = object({{"type", args["type"]}}); if (args.hasProperty("name")) data.getDynamicObject()->setProperty("niceName", args["name"]); auto* item = manager->mcpCreate(data); if (!item) throw Error(-32602, "Manager could not create this type"); return describe(handle(item)); }); },
        [this](const var& args, const Context& c) { requireRevision(args); auto* cc = container(args["handle"].toString()); auto* manager = dynamic_cast<ManagerAdapter*>(cc); if (!manager || !manager->mcpAcceptsType(args["type"].toString())) throw Error(-32602, "Unsupported manager type"); c.require(creationCapabilities(cc, args["type"].toString())); if (validateCreation) validateCreation(cc, args["type"].toString(), c); }, true);
    add("organic_remove", "Remove an item from its owning manager in one undo transaction.", editInput, edit,
        [this](const var& args, Context&) { return transaction("Remove object", [&] { auto* item = dynamic_cast<BaseItem*>(container(args["handle"].toString())); auto* manager = dynamic_cast<ManagerAdapter*>(item->parentContainer.get()); if (!manager->mcpRemove(item)) throw Error(-32602, "Removal rejected"); return object({{"removed", true}}); }); },
        [this](const var& args, const Context& c) { requireRevision(args); auto* item = dynamic_cast<BaseItem*>(container(args["handle"].toString())); if (!item || !item->userCanRemove || !dynamic_cast<ManagerAdapter*>(item->parentContainer.get())) throw Error(-32602, "Object cannot be removed"); auto nodes = item->getAllContainers(true); nodes.add(item); for (auto node : nodes) if (node && hasExecutableBehavior(node)) c.require(advanced); for (auto weak : item->getAllParameters(true)) if (auto* file = dynamic_cast<FileParameter*>(weak.get())) if (file->getValue().toString().isNotEmpty()) c.requiredPaths.addIfNotAlreadyThere(file->getFile().getFullPathName()); }, true);
    auto reorderInput = editInput.clone(); reorderInput["properties"].getDynamicObject()->setProperty("index", integerSchema(0, INT32_MAX)); reorderInput["required"].getArray()->add("index");
    add("organic_reorder", "Reorder an item within its owning manager.", reorderInput, edit,
        [this](const var& args, Context&) { return transaction("Reorder object", [&] { auto* item = dynamic_cast<BaseItem*>(container(args["handle"].toString())); auto* manager = dynamic_cast<ManagerAdapter*>(item->parentContainer.get()); if (!manager->mcpReorder(item, (int) args["index"])) throw Error(-32602, "Reorder rejected"); return describe(args["handle"].toString()); }); },
        [this](const var& args, const Context&) { requireRevision(args); auto* item = dynamic_cast<BaseItem*>(container(args["handle"].toString())); if (!item || !dynamic_cast<ManagerAdapter*>(item->parentContainer.get()) || (int) args["index"] >= item->parentContainer->controllableContainers.size()) throw Error(-32602, "Invalid reorder target"); }, true);
    auto moveInput = editInput.clone(); moveInput["properties"].getDynamicObject()->setProperty("destination", stringSchema()); moveInput["required"].getArray()->add("destination");
    moveInput["properties"].getDynamicObject()->setProperty("destinationRevision", integerSchema(1, INT32_MAX)); moveInput["required"].getArray()->add("destinationRevision");
    add("organic_move", "Move an item to a compatible destination manager.", moveInput, edit,
        [this](const var& args, Context&) { return transaction("Move object", [&] { auto* source = container(args["handle"].toString()); auto* destination = container(args["destination"].toString()); auto previous = destination->controllableContainers; juce::WeakReference<ControllableContainer> safeSource(source); if (!destination->handleMoveFromRemoteControl(source, true)) throw Error(-32602, "Destination rejected this item"); for (auto child : destination->controllableContainers) if (child && !previous.contains(child)) return describe(handle(child.get())); if (safeSource) return describe(handle(safeSource.get())); throw Error(-32000, "Move completed but the resulting object could not be resolved"); }); },
        [this](const var& args, const Context& c) { requireRevision(args); auto* source = container(args["handle"].toString()); auto* destination = container(args["destination"].toString()); if (source == root || destination == source || source->containsContainer(destination) || !dynamic_cast<ManagerAdapter*>(destination)) throw Error(-32602, "Invalid move destination"); if ((juce::int64) describe(args["destination"].toString())["revision"] != (juce::int64) args["destinationRevision"]) throw Error(-32006, "Destination changed since inspection"); auto nodes = source->getAllContainers(true); nodes.add(source); for (auto node : nodes) if (node && hasExecutableBehavior(node)) c.require(advanced); for (auto weak : source->getAllParameters(true)) if (auto* file = dynamic_cast<FileParameter*>(weak.get())) if (file->getValue().toString().isNotEmpty()) c.requireFile(file->getFile()); }, true);
    add("organic_duplicate", "Duplicate a manager item. Embedded executable behavior and referenced files retain their grant checks.", editInput, edit,
        [this](const var& args, Context&) { return transaction("Duplicate object", [&] { auto* item = dynamic_cast<BaseItem*>(container(args["handle"].toString())); auto* manager = dynamic_cast<ManagerAdapter*>(item->parentContainer.get()); auto* copy = manager->mcpCreate(item->getJSONData()); if (!copy) throw Error(-32602, "Duplication rejected"); return describe(handle(copy)); }); },
        [this](const var& args, const Context& c) { requireRevision(args); auto* item = dynamic_cast<BaseItem*>(container(args["handle"].toString())); if (!item || !item->userCanDuplicate || !dynamic_cast<ManagerAdapter*>(item->parentContainer.get())) throw Error(-32602, "Object cannot be duplicated"); auto nodes = item->getAllContainers(true); nodes.add(item); for (auto node : nodes) if (node && hasExecutableBehavior(node)) c.require(advanced); for (auto weak : item->getAllParameters(true)) if (auto* f = dynamic_cast<FileParameter*>(weak.get())) if (f->getValue().toString().isNotEmpty()) c.requireFile(juce::File(f->absolutePath)); }, true);
    for (auto redo : {false, true})
        add(redo ? "organic_redo" : "organic_undo", "Undo or redo the specified MCP transaction; intervening history causes a conflict.", schema(object({{"transaction", stringSchema()}}), array({"transaction"})), edit,
            [this, redo](const var& args, Context&) { auto* undo = UndoMaster::getInstance(); if (!(redo ? undo->redo() : undo->undo())) throw Error(-32006, "Undo history changed"); return object({{"transaction", args["transaction"]}, {"redone", redo}}); },
            [this, redo](const var& args, const Context& c) { ensureReady(); auto it = transactions.find(args["transaction"].toString()); auto* undo = UndoMaster::getInstance(); if (it == transactions.end() || (redo ? undo->getRedoDescription() : undo->getUndoDescription()) != it->second.title) throw Error(-32006, "Requested transaction is not at the current history position"); c.require(it->second.capabilities); for (const auto& path : it->second.paths) c.requireFile(juce::File(path)); }, true);
    auto itemSchema = schema(object({{"name", stringSchema()}, {"arguments", openObjectSchema()}}), array({"name", "arguments"}));
    auto batchSchema = schema(object({{"operations", object({{"type", "array"}, {"items", itemSchema}, {"minItems", 1}, {"maxItems", 64}})}, {"preview", object({{"type", "boolean"}})}}), array({"operations"}));
    add("organic_batch", "Prevalidate up to 64 operations, optionally preview, then execute as one named undo group. Failures report completed operations and irreversible effects.", batchSchema, inspect,
        [this](const var& args, Context& c)
        {
            if ((bool) args["preview"]) return object({{"valid", true}, {"operationCount", args["operations"].size()}, {"preview", true}});
            return transaction("Batch edit", [&]
            {
                juce::Array<var> completed; bool irreversible = false;
                for (const auto& operation : *args["operations"].getArray())
                {
                    auto name = operation["name"].toString();
                    try
                    {
                        auto result = registry.call(name, operation["arguments"], c);
                        irreversible |= registry.find(name).externalEffects || (!registry.find(name).reversible && registry.find(name).capabilities != inspect);
                        completed.add(object({{"name", name}, {"result", result}}));
                    }
                    catch (const Error& error)
                    {
                        return object({{"success", false}, {"completed", completed}, {"failedOperation", name}, {"error", String(error.what())},
                            {"irreversibleEffects", irreversible}, {"recovery", "Inspect completed results and current revisions before retrying. Undo the returned transaction only if it remains at the current history position."}});
                    }
                }
                return object({{"success", true}, {"completed", completed}, {"irreversibleEffects", irreversible}});
            });
        },
        [this](const var& args, const Context& c)
        {
            for (const auto& operation : *args["operations"].getArray())
            {
                auto name = operation["name"].toString();
                if (name == "organic_batch" || name == "organic_undo" || name == "organic_redo" || name == "chataigne_new" || name == "chataigne_open")
                    throw Error(-32602, "History and project replacement operations cannot be nested in a batch");
                registry.prevalidate(name, operation["arguments"], c);
            }
        }, true);
    registry.add(Resource{"organic://identity", "identity", "Application identity", [](Context&) { return object({{"name", juce::JUCEApplicationBase::getInstance() ? juce::JUCEApplicationBase::getInstance()->getApplicationName() : "OrganicUI"}}); }});
    registry.add(Resource{"organic://project", "project", "Project root summary", [this](Context&) { return describe(handle(root.get())); }});
    registry.add(Resource{"organic://capabilities", "capabilities", "Operation schemas and capabilities", [this](Context&) { return registry.tools(); }});
    registry.add(ResourceTemplate{"organic://object/{handle}", "object_details", "Current object details, revisions and parameter constraints for a discovered handle", [this](const String& id, Context&) { return describe(id); }});
    registry.add(Resource{"organic://usage", "usage", "OrganicUI authoring and access guide", [](Context&)
    {
        return object({{"workflow", array({"Start with organic_browse and follow scoped handles and nextOffset pages.",
            "Inspect targets and discover manager types or controllableTypes before creating or configuring objects.",
            "Use the latest revision with typed edits; report returned undo transaction IDs.",
            "Preview batches to validate them; inspect completed results and recovery instructions after partial failures.",
            "Refresh discovery after project replacement. Old handles and transactions belong to the previous generation."})},
            {"objectDetails", "organic_inspect returns resourceUri; resources/templates/list advertises organic://object/{handle}. Reads use the same redaction and stale-handle checks."},
            {"permissions", "Authenticated credentials select remembered Inspect, Control, Edit or Full access. Code authoring and direct script/system execution require Advanced. Full includes all supported actions and accessible paths; Advanced is not a filesystem sandbox."},
            {"files", "Restricted file operations require approved absolute paths. Arbitrary serialized configuration requires Full and may have irreversible external effects."},
            {"observation", "Use organic_observe to select parameters, then subscribe to organic://observation. Sampling and retained history are bounded."},
            {"diagnostics", "Inspect observed warnings, targets and values. Sending output or executing a test command is a separate control action."}});
    }});
    add("organic_observe", "Choose up to 32 parameter handles for this credential's bounded observation history. Subscribe to the returned resource to watch changes.",
        schema(object({{"handles", object({{"type", "array"}, {"items", stringSchema()}, {"maxItems", 32}})}}), array({"handles"})), inspect,
        [this](const var& args, Context& c)
        {
            if (observations.size() >= 128 && !observations.count(c.grant.id)) throw Error(-32000, "Observation limit reached");
            Observation observation; for (const auto& id : *args["handles"].getArray()) observation.handles.add(id.toString());
            observations[c.grant.id] = observation;
            return object({{"uri", "organic://observation"}, {"sampleIntervalMs", 200}, {"retainedSamples", 64}});
        }, [this](const var& args, const Context&) { for (const auto& id : *args["handles"].getArray()) if (!dynamic_cast<Parameter*>(controllable(id.toString()))) throw Error(-32602, "Observe parameter handles only"); });
    registry.add(Resource{"organic://observation", "observations", "This credential's selected parameter samples", [this](Context& c)
    {
        ensureReady(); auto found = observations.find(c.grant.id);
        if (found == observations.end()) return object({{"samples", array({})}});
        auto& observation = found->second; auto now = juce::Time::getMillisecondCounterHiRes();
        if (now - observation.lastTime >= 200)
        {
            juce::Array<var> values;
            for (const auto& id : observation.handles)
            {
                try
                {
                    auto info = describe(id); auto value = info["value"];
                    if (value.isString()) value = value.toString().substring(0, 128);
                    values.add(object({{"handle", id}, {"value", value}, {"revision", info["revision"]}}));
                }
                catch (const Error&) { values.add(object({{"handle", id}, {"deleted", true}})); }
            }
            observation.samples.push_back(object({{"time", juce::Time::getCurrentTime().toISO8601(true)}, {"values", values}}));
            if (observation.samples.size() > 64) observation.samples.pop_front(); observation.lastTime = now;
        }
        juce::Array<var> samples; for (const auto& sample : observation.samples) samples.add(sample);
        return object({{"samples", samples}, {"generation", documentGeneration}});
    }});
    registry.add(Prompt{"organic_build_project", "Author a project", "Browse the project root and available types. Inspect each target and its revision before editing. Use the smallest supported authoring operations, validate references, then save. Report external effects and undo transaction IDs."});
    registry.add(Prompt{"organic_explain_signal_flow", "Explain signal flow", "Inspect the relevant inputs, processing containers, target references, and outputs. Explain the observed connections with object handles and evidence. Keep diagnostics read-only."});
}
}
