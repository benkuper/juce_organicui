#pragma once
class BaseItem;
class ControllableContainer;

namespace organic::mcp
{
// Runtime interface for existing templated managers. Permission checks belong to
// registry operations, never to ad-hoc remote-control entry points.
class ManagerAdapter
{
public:
    virtual ~ManagerAdapter() = default;
    virtual ControllableContainer& mcpContainer() = 0;
    virtual juce::var mcpTypes() const = 0;
    virtual bool mcpAcceptsType(const juce::String&) = 0;
    virtual BaseItem* mcpCreate(juce::var data) = 0;
    virtual bool mcpRemove(BaseItem*) = 0;
    virtual bool mcpReorder(BaseItem*, int index) = 0;
};
}
