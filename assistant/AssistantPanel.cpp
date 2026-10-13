#include "AssistantPanel.h"
namespace organic::assistant
{
class Panel::Conversation : public juce::Component
{
public:
    struct Entry { String role, text, detail; bool expanded = false, error = false; juce::Rectangle<float> bounds; juce::TextLayout layout; };
    std::vector<Entry> entries;
    void update(const String& transcript)
    {
        std::vector<Entry> next;
        for (auto line : juce::StringArray::fromLines(transcript))
        {
            if (line.startsWith("You: ")) next.push_back({"YOU", line.substring(5), {}});
            else if (line.startsWith("Assistant:")) next.push_back({"ASSISTANT", line.substring(10).trim(), {}});
            else if (line.startsWith("[Tool: "))
            {
                auto name = line.substring(7).upToFirstOccurrenceOf("]", false, false);
                Entry entry {"TOOL", name, line.fromFirstOccurrenceOf("]", false, false).trim()};
                try
                {
                    auto result = mcp::parseBounded(entry.detail);
                    entry.error = result.hasProperty("error") || (result.hasProperty("success") && !(bool) result["success"]);
                    if (entry.error) entry.text += "\n" + (result["message"].toString().isEmpty() ? String("Partial execution - review details") : result["message"].toString());
                    else if (result.hasProperty("transaction")) entry.text += "\nApplied - undo transaction " + result["transaction"].toString();
                    else entry.text += "\nCompleted";
                    if ((bool) result["irreversibleEffects"] || (result.hasProperty("reversible") && !(bool) result["reversible"])) entry.text += "\nExternal effects cannot be undone";
                }
                catch (...) { entry.text += "\nResult available"; }
                next.push_back(std::move(entry));
            }
            else if (!next.empty()) { if (!next.back().text.isEmpty()) next.back().text += "\n"; next.back().text += line; }
        }
        for (size_t i = 0; i < next.size() && i < entries.size(); ++i) if (next[i].role == entries[i].role) next[i].expanded = entries[i].expanded;
        for (size_t i = 0; i + 1 < next.size();) { if (next[i].role == "ASSISTANT" && next[i].text.trim().isEmpty()) next.erase(next.begin() + (ptrdiff_t) i); else ++i; }
        entries = std::move(next); relayout();
    }
    void relayout()
    {
        const auto width = juce::jmax(80, juce::jmin(840, getWidth() - 32));
        float y = 16;
        const auto fontSize = (float) juce::jlimit(12, 20, GlobalSettings::getInstance()->fontSize->intValue() + 2);
        for (auto& entry : entries)
        {
            juce::AttributedString text;
            auto content = entry.text.trimEnd(); if (entry.expanded && !entry.detail.isEmpty()) content += "\n\n" + entry.detail.substring(0, 12000);
            text.append(content, juce::Font(entry.role == "TOOL" ? fontSize - 1 : fontSize), entry.error ? RED_COLOR.brighter(.6f) : TEXT_COLOR);
            entry.layout.createLayout(text, (float) juce::jmax(60, width - 32));
            const auto h = juce::jmax(54.f, entry.layout.getHeight() + 46.f);
            entry.bounds = {(float) (getWidth() - width) / 2, y, (float) width, h}; y += h + 12;
        }
        setSize(getWidth(), (int) y + 8); repaint();
    }
    void resized() override { relayout(); }
    void paint(juce::Graphics& g) override
    {
        for (auto& entry : entries)
        {
            auto r = entry.bounds;
            g.setColour(entry.role == "YOU" ? PANEL_COLOR.darker(.35f) : (entry.role == "TOOL" ? BG_COLOR.brighter(.07f) : BG_COLOR.brighter(.025f)));
            g.fillRoundedRectangle(r, 5.f);
            g.setColour(entry.error ? RED_COLOR : (entry.role == "YOU" ? HIGHLIGHT_COLOR : (entry.role == "TOOL" ? NORMAL_COLOR : FEEDBACK_COLOR)));
            g.fillRoundedRectangle(r.getX(), r.getY() + 14, 3, r.getHeight() - 28, 1.f);
            g.setFont(juce::Font(10.f, juce::Font::bold));
            g.drawText(entry.role == "TOOL" ? "TOOL ACTIVITY - click for details" : entry.role, r.reduced(16).removeFromTop(14), juce::Justification::left);
            entry.layout.draw(g, r.reduced(16).withTrimmedTop(18));
        }
    }
    void mouseUp(const juce::MouseEvent& event) override
    {
        for (auto& entry : entries) if (entry.role == "TOOL" && entry.bounds.contains(event.position))
        { entry.expanded = !entry.expanded; relayout(); return; }
    }
};
Panel::Panel(const String& name, Controller& controller) : ShapeShifterContentComponent(name), controller(controller), owner(&controller)
{
    setOpaque(true); conversation = std::make_unique<Conversation>();
    addAndMakeVisible(viewport); viewport.setViewedComponent(conversation.get(), false); viewport.setScrollBarsShown(true, false);
    composer.reset(controller.prompt->createStringParameterTextUI()); composer->editor.setName("Assistant message");
    composer->editor.setColour(juce::TextEditor::backgroundColourId, BG_COLOR.darker(.15f));
    composer->editor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    composer->editor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    composer->editor.setTextToShowWhenEmpty("Describe what you want to build or change...", TEXTNAME_COLOR);
    composer->editor.addKeyListener(this); addAndMakeVisible(composer.get());
    model.reset(controller.models->createUI()); addAndMakeVisible(model.get());
    auto button = [this](Trigger* trigger)
    {
        auto ui = std::unique_ptr<TriggerButtonUI>(trigger->createButtonUI());
        ui->customTextSize = 12; addAndMakeVisible(ui.get()); return ui;
    };
    send = button(controller.send); send->useCustomBGColor = true; send->customBGColor = HIGHLIGHT_COLOR.darker(.25f);
    cancelButton = button(controller.cancelRequest); clear = button(controller.clearHistory); settings = button(controller.openSettings); signIn = button(controller.signIn);
    for (auto* trigger : {controller.explainProject, controller.findIssues, controller.buildWorkflow}) suggestions.add(button(trigger).release());
    timerCallback(); startTimerHz(5);
}
Panel::~Panel() { stopTimer(); composer->editor.removeKeyListener(this); viewport.setViewedComponent(nullptr, false); }
bool Panel::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (owner == nullptr) return false;
    if (key.getKeyCode() == juce::KeyPress::returnKey && (key.getModifiers().isCtrlDown() || key.getModifiers().isCommandDown()))
    { controller.send->trigger(); return true; } return false;
}
void Panel::timerCallback()
{
    if (owner == nullptr) { stopTimer(); return; }
    controller.refresh(); auto view = controller.provider.view();
    isBusy = view.busy; hasAccount = !view.active.isEmpty(); statusText = controller.status->stringValue();
    accountLabel = hasAccount ? controller.accounts->getValueKey() : "ChatGPT plan (preview)";
    if (displayedTranscript != view.transcript)
    {
        const bool follow = viewport.getViewPositionY() + viewport.getHeight() >= conversation->getHeight() - 50;
        displayedTranscript = view.transcript; conversation->update(displayedTranscript);
        if (follow) viewport.setViewPosition(0, juce::jmax(0, conversation->getHeight() - viewport.getHeight()));
    }
    cancelButton->setVisible(isBusy); send->setVisible(!isBusy);
    signIn->setVisible(view.models.size() == 0 && !view.busy);
    for (auto* suggestion : suggestions) suggestion->setVisible(displayedTranscript.trim().isEmpty());
    repaint(); resized();
}
void Panel::paint(juce::Graphics& g)
{
    g.fillAll(BG_COLOR);
    g.setColour(PANEL_COLOR.darker(.3f)); g.fillRect(header);
    auto r = header.reduced(16, 9); g.setColour(TEXT_COLOR); g.setFont(juce::Font(18.f, juce::Font::bold));
    g.drawText("Assistant", r.removeFromTop(24), juce::Justification::left);
    g.setFont(11.f); g.setColour(TEXTNAME_COLOR); g.drawFittedText(accountLabel, r.withWidth(juce::jmin(280, r.getWidth())), juce::Justification::left, 1);
    g.setColour(CONTOUR_COLOR); g.drawHorizontalLine(header.getBottom() - 1, 0.f, (float) getWidth());
    g.setColour(PANEL_COLOR.darker(.2f)); g.fillRoundedRectangle(composerCard.toFloat(), 6.f);
    g.setColour(isBusy ? HIGHLIGHT_COLOR : NORMAL_COLOR); g.fillEllipse((float) footer.getX(), (float) footer.getY() + 5, 5.f, 5.f);
    g.setColour(TEXTNAME_COLOR); g.setFont(11.f); g.drawFittedText(statusText, footer.withTrimmedLeft(12), juce::Justification::topLeft, 3);
    if (displayedTranscript.trim().isEmpty())
    {
        auto area = emptyState; g.setColour(TEXT_COLOR); g.setFont(juce::Font(getWidth() < 460 ? 18.f : 24.f, juce::Font::bold));
        g.drawFittedText("What would you like to build?", area.removeFromTop(58), juce::Justification::centred, 2);
        g.setFont(13.f); g.setColour(TEXTNAME_COLOR);
        g.drawFittedText("Explore your project, diagnose a connection,\nor describe a workflow to create.", area.removeFromTop(50), juce::Justification::centred, 3);
    }
}
void Panel::resized()
{
    auto area = getLocalBounds(); const bool narrow = getWidth() < 500;
    header = area.removeFromTop(narrow ? 132 : 98);
    auto actions = header.reduced(12, 9).removeFromBottom(28);
    settings->setBounds(actions.removeFromRight(112)); actions.removeFromRight(6); clear->setBounds(actions.removeFromRight(120));
    model->setBounds(actions.reduced(0, 2));
    if (narrow) model->setBounds(header.reduced(12, 9).withY(64).withHeight(26));
    area.reduce(narrow ? 10 : 20, 10);
    footer = area.removeFromBottom(44); area.removeFromBottom(6);
    const int composerHeight = juce::jlimit(100, 160, getHeight() / 5);
    composerCard = area.removeFromBottom(juce::jmin(composerHeight, area.getHeight() / 2));
    auto input = composerCard.reduced(8); auto controls = input.removeFromBottom(28);
    send->setBounds(controls.removeFromRight(78)); cancelButton->setBounds(send->getBounds());
    signIn->setBounds(controls.removeFromLeft(juce::jmin(170, controls.getWidth())).reduced(0, 2));
    composer->setBounds(input.withTrimmedBottom(6)); area.removeFromBottom(10); viewport.setBounds(area);
    const int available = juce::jmax(80, viewport.getWidth() - viewport.getScrollBarThickness());
    if (conversation->getWidth() != available) conversation->setSize(available, conversation->getHeight());
    emptyState = area.withSizeKeepingCentre(juce::jmin(600, area.getWidth()), juce::jmin(250, area.getHeight()));
    auto chips = emptyState.withTrimmedTop(118);
    if (narrow) for (auto* suggestion : suggestions) suggestion->setBounds(chips.removeFromTop(30).reduced(8, 2));
    else for (auto* suggestion : suggestions) suggestion->setBounds(chips.removeFromLeft(emptyState.getWidth() / 3).withHeight(30).reduced(4, 2));
}
}
