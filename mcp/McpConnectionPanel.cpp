#include "McpConnectionPanel.h"
namespace organic::mcp
{
ConnectionPanel::ConnectionPanel(const String& name, ConnectionSettings& model, BoolParameter& enabled, IntParameter& port)
    : ShapeShifterContentComponent(name), settings(model), owner(&model)
{
    setOpaque(true); addAndMakeVisible(viewport);
    settings.activity->hideInEditor = true;
    editor = std::make_unique<GenericControllableContainerEditor>(juce::Array<ControllableContainer*>{&settings}, true);
    viewport.setViewedComponent(editor.get(), false); viewport.setScrollBarsShown(true, false);
    enabledUI.reset(enabled.createDefaultUI()); portUI.reset(port.createDefaultUI());
    addAndMakeVisible(enabledUI.get()); addAndMakeVisible(portUI.get());
    activityUI.reset(settings.activity->createStringParameterTextUI()); addAndMakeVisible(activityUI.get());
    activityUI->editor.setColour(juce::TextEditor::backgroundColourId, BG_COLOR.darker(.1f));
    activityUI->editor.setReadOnly(true); activityUI->editor.setEnabled(true);
    startTimerHz(2);
}
ConnectionPanel::~ConnectionPanel() { stopTimer(); viewport.setViewedComponent(nullptr, false); }
void ConnectionPanel::paint(juce::Graphics& g)
{
    g.fillAll(BG_COLOR); g.setColour(PANEL_COLOR.darker(.3f)); g.fillRect(header);
    if (owner == nullptr) return;
    auto title = header.reduced(12, 8); g.setColour(TEXT_COLOR); g.setFont(juce::Font(18.f, juce::Font::bold));
    g.drawText("MCP connections", title.removeFromTop(24), juce::Justification::left);
    g.setFont(11.f); g.setColour(settings.server.isRunning() ? FEEDBACK_COLOR : TEXTNAME_COLOR);
    g.drawFittedText((settings.server.isRunning() ? "Listening  " : "Endpoint  ") + settings.endpoint(), title.removeFromTop(22), juce::Justification::left, 1);
    g.setColour(TEXTNAME_COLOR); g.setFont(11.f);
    g.drawText("RECENT ACTIVITY", activityBounds.withHeight(20), juce::Justification::left);
}
void ConnectionPanel::resized()
{
    auto area = getLocalBounds(); header = area.removeFromTop(94); auto controls = header.reduced(12, 8).removeFromBottom(24);
    portUI->setBounds(controls.removeFromRight(110)); controls.removeFromRight(8); enabledUI->setBounds(controls.removeFromLeft(juce::jmin(240, controls.getWidth())));
    area.reduce(10, 10);
    if (getWidth() >= 780)
    { activityBounds = area.removeFromRight(juce::jmax(280, area.getWidth() / 3)); area.removeFromRight(16); }
    else { activityBounds = area.removeFromBottom(juce::jmin(150, getHeight() / 4)); area.removeFromBottom(12); }
    activityUI->setBounds(activityBounds.withTrimmedTop(24)); viewport.setBounds(area);
    editor->setSize(juce::jmax(100, viewport.getWidth() - viewport.getScrollBarThickness()), editor->getHeight());
}
}
