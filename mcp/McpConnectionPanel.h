#pragma once
#include "McpConnectionSettings.h"
namespace organic::mcp
{
class ConnectionPanel final : public ShapeShifterContentComponent, private juce::Timer
{
public:
    ConnectionPanel(const String& name, ConnectionSettings&, BoolParameter& enabled, IntParameter& port);
    ~ConnectionPanel() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    ConnectionSettings& settings;
    juce::WeakReference<ControllableContainer> owner;
    juce::Viewport viewport;
    std::unique_ptr<GenericControllableContainerEditor> editor;
    std::unique_ptr<ControllableUI> enabledUI, portUI;
    std::unique_ptr<StringParameterTextUI> activityUI;
    juce::Rectangle<int> header, activityBounds;
    void timerCallback() override { repaint(); }
};
}
