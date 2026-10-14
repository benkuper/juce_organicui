#include "AssistantPanel.h"
namespace organic::assistant
{
namespace
{
String actionLabel(const String& name)
{
    if (name == "assistant_task") return "Checked requested work";
    if (name == "organic_batch") return "Applied grouped changes";
    if (name == "organic_browse" || name == "organic_search") return "Explored project";
    if (name == "organic_inspect" || name == "organic_types") return "Inspected settings";
    if (name == "organic_create" || name == "organic_add_controllable") return "Created object";
    if (name == "organic_set_value" || name == "organic_set_range") return "Updated settings";
    if (name == "organic_remove") return "Removed object";
    if (name == "organic_rename") return "Renamed object";
    if (name.contains("diagnostic")) return "Checked project";
    if (name.contains("save")) return "Saved project";
    if (name.contains("command")) return "Configured command";
    return "Project action";
}
void appendMarkdown(juce::AttributedString& output, const String& source, float size, juce::Colour colour)
{
    bool code = false, first = true;
    for (auto line : juce::StringArray::fromLines(source))
    {
        if (line.trimStart().startsWith("```")) { code = !code; continue; }
        auto font = code ? juce::Font(juce::Font::getDefaultMonospacedFontName(), size - 1, juce::Font::plain) : juce::Font(size);
        if (!code && line.startsWith("#"))
        {
            auto heading = line.indexOfChar(' ');
            if (heading > 0 && heading <= 6) { line = line.substring(heading + 1); font = juce::Font(size + 1, juce::Font::bold); }
        }
        if (!code && line.startsWith("- ")) line = String::fromUTF8("\xe2\x80\xa2 ") + line.substring(2);
        if (!first) output.append("\n", font, colour);
        first = false;
        while (!code && (line.contains("**") || line.contains("`")))
        {
            int bold = line.indexOf("**"), mono = line.indexOfChar('`');
            bool isBold = bold >= 0 && (mono < 0 || bold < mono);
            int begin = isBold ? bold : mono; auto marker = isBold ? String("**") : String("`");
            int end = line.indexOf(begin + marker.length(), marker);
            if (end < 0) break;
            output.append(line.substring(0, begin), font, colour);
            auto emphasis = isBold ? font.boldened() : juce::Font(juce::Font::getDefaultMonospacedFontName(), size - 1, juce::Font::plain);
            output.append(line.substring(begin + marker.length(), end), emphasis, colour);
            line = line.substring(end + marker.length());
        }
        output.append(line, font, colour);
    }
    output.setLineSpacing(3.f);
}
}
class Panel::IconButton : public juce::Button
{
public:
    enum Kind { history, settings, send, stop, plus, trash, back, copy };
    IconButton(Kind kind, const String& label, std::function<void()> action) : juce::Button(label), kind(kind)
    { setTooltip(label); setTitle(label); onClick = std::move(action); }
    void setKind(Kind next) { kind = next; repaint(); }
    void paintButton(juce::Graphics& g, bool hover, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(2);
        if (kind == send || hover || down)
        { g.setColour(kind == send ? HIGHLIGHT_COLOR.withAlpha(isEnabled() ? .8f : .2f) : TEXT_COLOR.withAlpha(down ? .12f : .06f)); g.fillRoundedRectangle(r, 5); }
        g.setColour(TEXT_COLOR.withAlpha(isEnabled() ? .9f : .3f));
        auto centre = r.getCentre(); g.addTransform(juce::AffineTransform::translation(centre.x - 8, centre.y - 8));
        juce::Path path;
        if (kind == history)
        { path.addArc(2, 2, 12, 12, -.7f, 4.8f, true); path.startNewSubPath(1, 1); path.lineTo(1, 6); path.lineTo(6, 6); path.startNewSubPath(8, 4); path.lineTo(8, 8); path.lineTo(11, 9); }
        else if (kind == settings)
        { for (int x : {3, 8, 13}) { path.startNewSubPath((float) x, 2); path.lineTo((float) x, 14); } path.addEllipse(1, 5, 4, 4); path.addEllipse(6, 9, 4, 4); path.addEllipse(11, 3, 4, 4); }
        else if (kind == send)
        { path.startNewSubPath(8, 13); path.lineTo(8, 3); path.startNewSubPath(3, 8); path.lineTo(8, 3); path.lineTo(13, 8); }
        else if (kind == stop) path.addRoundedRectangle(3, 3, 10, 10, 1);
        else if (kind == plus)
        { path.startNewSubPath(8, 2); path.lineTo(8, 14); path.startNewSubPath(2, 8); path.lineTo(14, 8); }
        else if (kind == back)
        { path.startNewSubPath(13, 8); path.lineTo(3, 8); path.startNewSubPath(8, 3); path.lineTo(3, 8); path.lineTo(8, 13); }
        else if (kind == copy)
        { path.addRoundedRectangle(5, 5, 9, 9, 1); path.startNewSubPath(3, 11); path.lineTo(2, 11); path.lineTo(2, 2); path.lineTo(11, 2); path.lineTo(11, 3); }
        else
        { path.startNewSubPath(2, 4); path.lineTo(14, 4); path.addRectangle(4, 4, 8, 10); path.startNewSubPath(6, 2); path.lineTo(10, 2); }
        g.strokePath(path, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
private:
    Kind kind;
};
class Panel::Conversation : public juce::Component
{
public:
    struct Entry
    {
        String role, text, detail, plainText;
        int count = 0, errors = 0;
        bool expanded = false;
        juce::Rectangle<float> bounds;
        juce::TextLayout layout;
    };
    std::vector<Entry> entries;
    Conversation() : messageCopy(IconButton::copy, "Copy message", [this]
    {
        if (hoveredEntry < 0 || hoveredEntry >= (int) entries.size()) return;
        juce::SystemClipboard::copyTextToClipboard(entries[(size_t) hoveredEntry].plainText);
        messageCopy.setTooltip("Message copied");
    })
    {
        setName("Conversation messages"); addChildComponent(messageCopy);
        messageCopy.addMouseListener(this, false);
    }
    ~Conversation() override { messageCopy.removeMouseListener(this); }
    void update(const String& transcript)
    {
        std::vector<Entry> parsed, next;
        for (auto line : juce::StringArray::fromLines(transcript))
        {
            if (line.startsWith("You: ")) parsed.push_back({"YOU", line.substring(5), {}});
            else if (line.startsWith("Assistant:")) parsed.push_back({"ASSISTANT", line.substring(10).trim(), {}});
            else if (line.startsWith("[Status: "))
            { Entry entry {"STATUS", line.substring(9).upToFirstOccurrenceOf("]", false, false) + ": " + line.fromFirstOccurrenceOf("]", false, false).trim(), {}}; entry.errors = line.startsWith("[Status: Failed]"); parsed.push_back(std::move(entry)); }
            else if (line.startsWith("[Tool: "))
            {
                auto name = line.substring(7).upToFirstOccurrenceOf("]", false, false);
                Entry entry {"TOOL", actionLabel(name), {}}; entry.count = 1;
                auto detail = line.fromFirstOccurrenceOf("]", false, false).trim();
                try
                {
                    auto result = mcp::parseBounded(detail);
                    entry.errors = result.hasProperty("error") || (result.hasProperty("success") && !(bool) result["success"]);
                    if (entry.errors) entry.detail = result["message"].toString().isEmpty() ? "Action failed" : result["message"].toString();
                    else if (!result["name"].toString().isEmpty()) entry.detail = result["name"].toString();
                }
                catch (...) { entry.detail = "Working..."; }
                parsed.push_back(std::move(entry));
            }
            else if (!parsed.empty() && parsed.back().role != "TOOL")
            { if (!parsed.back().text.isEmpty()) parsed.back().text += "\n"; parsed.back().text += line; }
        }
        for (auto& entry : parsed)
        {
            if (entry.role == "ASSISTANT" && entry.text.trim().isEmpty()) continue;
            if (entry.role == "TOOL")
            {
                String detail = entry.text + (entry.detail.isEmpty() ? String() : " - " + entry.detail);
                if (!next.empty() && next.back().role == "TOOL")
                { auto& group = next.back(); group.count += entry.count; group.errors += entry.errors; group.detail += "\n" + detail; }
                else { entry.detail = detail; next.push_back(std::move(entry)); }
            }
            else next.push_back(std::move(entry));
        }
        for (size_t i = 0; i < next.size(); ++i)
        {
            auto& entry = next[i];
            if (entry.role == "TOOL") entry.text = String(entry.count) + (entry.count == 1 ? " project action" : " project actions")
                + (entry.errors > 0 ? " - " + String(entry.errors) + (entry.errors == 1 ? " needs attention" : " need attention") : String());
            if (i < entries.size() && entry.role == entries[i].role) entry.expanded = entries[i].expanded;
        }
        entries = std::move(next); relayout();
    }
    void relayout()
    {
        float width = (float) juce::jmax(60, juce::jmin(820, getWidth() - 24)), y = 8;
        float size = (float) juce::jlimit(12, 16, GlobalSettings::getInstance()->fontSize->intValue());
        for (auto& entry : entries)
        {
            juce::AttributedString text;
            bool tool = entry.role == "TOOL";
            if (tool) text.append(entry.expanded ? entry.detail : entry.text, juce::Font(11.5f), entry.errors > 0 ? RED_COLOR.brighter(.5f) : TEXTNAME_COLOR);
            else appendMarkdown(text, entry.text.trimEnd(), size, entry.errors > 0 ? RED_COLOR.brighter(.5f) : TEXT_COLOR);
            entry.plainText = tool ? entry.detail : text.getText();
            entry.layout.createLayout(text, juce::jmax(40.f, width - (tool ? 30.f : 20.f)));
            float h = tool && !entry.expanded ? 28.f : entry.layout.getHeight() + (tool ? 18.f : 32.f);
            entry.bounds = {((float) getWidth() - width) / 2, y, width, h}; y += h + (tool ? 4.f : 12.f);
        }
        setSize(getWidth(), (int) y + 4); updateHover(pointerPosition); repaint();
    }
    void resized() override { relayout(); }
    void paint(juce::Graphics& g) override
    {
        for (auto& entry : entries)
        {
            auto r = entry.bounds;
            if (entry.role == "TOOL")
            {
                g.setColour(TEXTNAME_COLOR); g.setFont(12.f);
                g.drawText(entry.expanded ? "v" : ">", r.withWidth(12).withHeight(28), juce::Justification::centred);
                entry.layout.draw(g, r.withTrimmedLeft(18).withTrimmedTop(entry.expanded ? 8.f : 6.f));
                continue;
            }
            if (entry.role == "YOU") { g.setColour(PANEL_COLOR.withAlpha(.55f)); g.fillRoundedRectangle(r, 6.f); }
            g.setColour(entry.role == "YOU" ? TEXTNAME_COLOR : FEEDBACK_COLOR); g.setFont(juce::Font(10.5f, juce::Font::bold));
            g.drawText(entry.role == "YOU" ? "You" : (entry.role == "STATUS" ? "Status" : "Assistant"), r.reduced(10, 6).withHeight(14), juce::Justification::left);
            entry.layout.draw(g, r.reduced(10, 0).withTrimmedTop(24));
        }
    }
    void mouseEnter(const juce::MouseEvent& event) override { updateHover(event.getEventRelativeTo(this).position); }
    void mouseMove(const juce::MouseEvent& event) override { updateHover(event.getEventRelativeTo(this).position); }
    void mouseExit(const juce::MouseEvent& event) override { updateHover(event.getEventRelativeTo(this).position); }
    void mouseUp(const juce::MouseEvent& event) override
    {
        if (event.eventComponent != this) return;
        for (auto& entry : entries) if (entry.role == "TOOL" && entry.bounds.contains(event.position))
        { entry.expanded = !entry.expanded; relayout(); return; }
    }
    String copyText() const
    {
        juce::StringArray blocks;
        for (auto& entry : entries)
            blocks.add((entry.role == "YOU" ? String("You") : entry.role == "TOOL" ? String("Project activity") : entry.role == "STATUS" ? String("Status") : String("Assistant"))
                + ":\n" + entry.plainText);
        return blocks.joinIntoString("\n\n");
    }
private:
    IconButton messageCopy;
    int hoveredEntry = -1;
    juce::Point<float> pointerPosition {-1, -1};
    void updateHover(juce::Point<float> position)
    {
        pointerPosition = position; int next = -1;
        for (size_t i = 0; i < entries.size(); ++i)
            if (entries[i].role != "TOOL" && entries[i].bounds.contains(position)) { next = (int) i; break; }
        if (hoveredEntry != next) messageCopy.setTooltip("Copy message");
        hoveredEntry = next; messageCopy.setVisible(next >= 0);
        if (next >= 0)
        {
            auto r = entries[(size_t) next].bounds.toNearestInt();
            messageCopy.setBounds(r.getRight() - 28, r.getY() + 1, 24, 24);
        }
    }
};
class Panel::History : public juce::Component
{
public:
    var rows = mcp::array({}); String active;
    std::function<void(const String&)> select;
    void update(const View& view)
    { rows = view.conversations; active = view.conversation; setSize(getWidth(), juce::jmax(80, rows.size() * 56 + 16)); repaint(); }
    void paint(juce::Graphics& g) override
    {
        if (rows.size() == 0)
        { g.setColour(TEXTNAME_COLOR); g.setFont(12.f); g.drawText("Your conversations will appear here.", getLocalBounds().reduced(12), juce::Justification::centredTop); }
        int y = 8;
        for (auto row : *rows.getArray())
        {
            auto r = juce::Rectangle<int>(8, y, juce::jmax(1, getWidth() - 16), 48); y += 56;
            g.setColour(row["id"].toString() == active ? HIGHLIGHT_COLOR.withAlpha(.12f) : PANEL_COLOR.withAlpha(.35f));
            g.fillRoundedRectangle(r.toFloat(), 5);
            g.setColour(TEXT_COLOR); g.setFont(12.5f); g.drawText(row["title"].toString(), r.reduced(10).withHeight(18), juce::Justification::left);
            g.setColour(TEXTNAME_COLOR); g.setFont(10.5f);
            auto created = (double) row["created"];
            auto label = created > 0 ? juce::Time((juce::int64) (created * 1000)).formatted("%d %b %Y, %H:%M") : String("Saved conversation");
            g.drawText(label, r.reduced(10, 0).withTrimmedTop(28), juce::Justification::left);
        }
    }
    void mouseUp(const juce::MouseEvent& event) override
    {
        int index = (event.y - 8) / 56;
        if (event.mods.isLeftButtonDown() && !event.mouseWasDraggedSinceMouseDown() && event.y >= 8 && (event.y - 8) % 56 < 48 && index >= 0 && index < rows.size() && select)
        { auto id = rows[index]["id"].toString(); select(id); }
    }
};
Panel::Panel(const String& name, Controller& controller) : ShapeShifterContentComponent(name), controller(controller), owner(&controller)
{
    setOpaque(true); conversation = std::make_unique<Conversation>(); history = std::make_unique<History>();
    history->setName("Conversation history list");
    history->select = [this](const String& id) { if (owner != nullptr) { this->controller.selectConversation(id); showHistory(false); } };
    addAndMakeVisible(viewport); viewport.setViewedComponent(conversation.get(), false); viewport.setScrollBarsShown(true, false); viewport.setScrollBarThickness(7);
    composer.reset(controller.prompt->createStringParameterTextUI()); composer->editor.setName("Assistant message");
    composer->editor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    composer->editor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    composer->editor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    composer->editor.setFont(13.f); composer->editor.setTextToShowWhenEmpty("Ask about your project...", TEXTNAME_COLOR);
    composer->editor.addKeyListener(this); addAndMakeVisible(composer.get());
    model.reset(controller.models->createUI()); access.reset(controller.access->createUI()); effort.reset(controller.effort->createUI());
    for (auto* ui : {model.get(), access.get(), effort.get()})
    { ui->useCustomBGColor = true; ui->customBGColor = BG_COLOR.brighter(.035f); ui->updateUIParams(); addAndMakeVisible(ui); }
    model->cb.setTextWhenNothingSelected("Choose model"); access->setTooltip("Access is remembered for this account.");
    auto icon = [this](IconButton::Kind kind, const String& label, std::function<void()> action)
    { auto button = std::make_unique<IconButton>(kind, label, std::move(action)); addAndMakeVisible(button.get()); return button; };
    send = icon(IconButton::send, "Send (Ctrl+Enter)", [this] { if (owner != nullptr) this->controller.send->trigger(); });
    stop = icon(IconButton::stop, "Stop", [this] { if (owner != nullptr) this->controller.cancelRequest->trigger(); });
    historyButton = icon(IconButton::history, "Conversation history", [this] { showHistory(!showingHistory); });
    settingsButton = icon(IconButton::settings, "Account and settings", [this] { if (owner != nullptr) this->controller.openSettings->trigger(); });
    newButton = icon(IconButton::plus, "New conversation", [this] { if (owner != nullptr) { this->controller.newConversation->trigger(); showHistory(false); } });
    deleteButton = icon(IconButton::trash, "Delete current conversation", [this] { if (owner != nullptr) this->controller.clearHistory->trigger(); });
    copyButton = icon(IconButton::copy, "Copy conversation", [this]
    { juce::SystemClipboard::copyTextToClipboard(conversationText()); copiedUntil = juce::Time::getMillisecondCounterHiRes() + 2000; timerCallback(); });
    usageButton.setTooltip("View your ChatGPT plan usage. This sign-in API does not provide a usage percentage.");
    usageButton.onClick = [this] { if (owner != nullptr) this->controller.usage->trigger(); };
    usageButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack); addAndMakeVisible(usageButton);
    reconnectButton.onClick = [this] { if (owner != nullptr) this->controller.refreshModels->trigger(); };
    reconnectButton.setTooltip("Retry connecting using your saved ChatGPT session."); addAndMakeVisible(reconnectButton);
    statusLabel.setFont(11.f); statusLabel.setColour(juce::Label::textColourId, TEXTNAME_COLOR);
    statusLabel.setBorderSize(juce::BorderSize<int>(0)); addAndMakeVisible(statusLabel);
    auto button = [this](Trigger* trigger)
    { auto ui = std::unique_ptr<TriggerButtonUI>(trigger->createButtonUI()); ui->customTextSize = 12; addAndMakeVisible(ui.get()); return ui; };
    signIn = button(controller.signIn);
    for (auto* trigger : {controller.explainProject, controller.findIssues, controller.buildWorkflow}) suggestions.add(button(trigger).release());
    addKeyListener(this); viewport.addKeyListener(this); timerCallback(); startTimerHz(5);
}
Panel::~Panel() { stopTimer(); removeKeyListener(this); viewport.removeKeyListener(this); composer->editor.removeKeyListener(this); viewport.setViewedComponent(nullptr, false); }
String Panel::conversationText() const
{
    if (owner == nullptr) return {};
    return "Chataigne assistant\nModel: " + controller.models->getValueKey() + "\nEffort: " + controller.effort->getValueKey()
        + "\nAccess: " + controller.access->getValueKey() + "\nStatus: " + controller.status->stringValue() + "\n\n" + conversation->copyText();
}
void Panel::showHistory(bool show)
{
    if (show) conversationScroll = viewport.getViewPositionY();
    showingHistory = show; viewport.setViewedComponent(show ? (juce::Component*) history.get() : conversation.get(), false);
    historyButton->setKind(show ? IconButton::back : IconButton::history);
    viewport.setViewPosition(0, show ? 0 : conversationScroll); timerCallback();
}
bool Panel::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (owner == nullptr) return false;
    if (showingHistory && key.getKeyCode() == juce::KeyPress::escapeKey) { showHistory(false); return true; }
    if (key.getKeyCode() == juce::KeyPress::returnKey && (key.getModifiers().isCtrlDown() || key.getModifiers().isCommandDown()))
    { controller.send->trigger(); return true; } return false;
}
void Panel::timerCallback()
{
    if (owner == nullptr) { stopTimer(); return; }
    controller.refresh(); auto view = controller.provider.view();
    isBusy = view.busy; signedIn = view.signedIn; activity = view.activity; statusText = controller.status->stringValue();
    auto label = activity == Activity::failed ? "Failed: " + statusText : statusText;
    statusLabel.setText(juce::Time::getMillisecondCounterHiRes() < copiedUntil ? String("Conversation copied") : label.upToFirstOccurrenceOf("\n", false, false), juce::dontSendNotification); statusLabel.setTooltip(label); statusLabel.setVisible(!showingHistory);
    auto transcript = view.transcript;
    if (!controller.currentError().isEmpty()) transcript += "\n[Status: Failed] " + controller.currentError();
    else if (activity == Activity::failed && statusText != "Some actions need attention" && !transcript.contains(statusText)) transcript += "\n[Status: Failed] " + statusText;
    if (displayedTranscript != transcript || displayedConversation != view.conversation)
    {
        bool switched = displayedConversation != view.conversation;
        bool follow = viewport.getViewPositionY() + viewport.getHeight() >= conversation->getHeight() - 50;
        displayedConversation = view.conversation; displayedTranscript = transcript; conversation->update(displayedTranscript);
        if (!showingHistory && (follow || switched)) viewport.setViewPosition(0, juce::jmax(0, conversation->getHeight() - viewport.getHeight()));
    }
    history->update(view);
    stop->setVisible(isBusy); send->setVisible(!isBusy && signedIn && !showingHistory);
    send->setEnabled(controller.send->enabled); signIn->setVisible(!signedIn && !isBusy && !showingHistory);
    reconnectButton.setVisible(signedIn && view.models.size() == 0 && !isBusy && !showingHistory);
    composer->setVisible(!showingHistory); model->setVisible(signedIn && view.models.size() > 0 && !showingHistory); access->setVisible(signedIn && view.models.size() > 0 && !showingHistory);
    effort->setVisible(signedIn && view.models.size() > 0 && !showingHistory); copyButton->setVisible(!showingHistory);
    newButton->setVisible(showingHistory); deleteButton->setVisible(showingHistory && !displayedTranscript.trim().isEmpty());
    newButton->setEnabled(true); deleteButton->setEnabled(!isBusy);
    usageButton.setVisible(!showingHistory && signedIn);
    historyButton->setTooltip(showingHistory ? "Back to conversation" : "Conversation history");
    for (auto* suggestion : suggestions) suggestion->setVisible(!showingHistory && displayedTranscript.trim().isEmpty() && signedIn);
    resized(); repaint();
}
void Panel::paint(juce::Graphics& g)
{
    g.fillAll(BG_COLOR);
    auto title = header.reduced(12, 0); title.removeFromRight(showingHistory ? 156 : 92);
    g.setColour(TEXT_COLOR); g.setFont(juce::Font(12.f, juce::Font::bold));
    g.drawText(showingHistory ? "Conversations" : "Assistant", title, juce::Justification::left);
    g.setColour(CONTOUR_COLOR.withAlpha(.6f)); g.drawHorizontalLine(header.getBottom() - 1, 0.f, (float) getWidth());
    if (showingHistory) return;
    g.setColour(BG_COLOR.brighter(.045f)); g.fillRoundedRectangle(composerCard.toFloat(), 7.f);
    g.setColour(CONTOUR_COLOR.withAlpha(.8f)); g.drawRoundedRectangle(composerCard.toFloat().reduced(.5f), 7.f, 1);
    auto colour = activity == Activity::failed ? RED_COLOR : (isBusy ? HIGHLIGHT_COLOR : TEXTNAME_COLOR);
    if (activity == Activity::completed) colour = FEEDBACK_COLOR;
    auto label = statusLine; auto dot = label.removeFromLeft(12).toFloat().withSizeKeepingCentre(5, 5);
    g.setColour(colour); g.fillEllipse(dot);
    if (displayedTranscript.trim().isEmpty())
    {
        auto area = emptyState; g.setColour(TEXT_COLOR); g.setFont(juce::Font(17.f, juce::Font::bold));
        g.drawFittedText(signedIn ? "What would you like to build?" : "Your project assistant", area.removeFromTop(42), juce::Justification::centred, 2);
        g.setFont(12.f); g.setColour(TEXTNAME_COLOR);
        g.drawFittedText(signedIn ? "Explore, troubleshoot, or create a workflow." : "Sign in once with ChatGPT to get started.", area.removeFromTop(38), juce::Justification::centred, 2);
    }
}
void Panel::resized()
{
    bool follow = viewport.getViewPositionY() + viewport.getHeight() >= conversation->getHeight() - 50;
    auto area = getLocalBounds(); header = area.removeFromTop(34);
    auto actions = header.reduced(6, 3);
    settingsButton->setBounds(actions.removeFromRight(28)); historyButton->setBounds(actions.removeFromRight(28));
    if (!showingHistory) copyButton->setBounds(actions.removeFromRight(28));
    if (showingHistory)
    { newButton->setBounds(actions.removeFromRight(28)); deleteButton->setBounds(actions.removeFromRight(28)); if (isBusy) stop->setBounds(actions.removeFromRight(28)); }
    area.reduce(8, 6);
    if (!showingHistory)
    {
        int lines = juce::jmax(1, composer->editor.getText().length() / juce::jmax(16, getWidth() / 7));
        lines = juce::jmax(lines, juce::StringArray::fromLines(composer->editor.getText()).size());
        int desired = signedIn ? juce::jlimit(82, 146, 64 + lines * 18) : 92;
        composerCard = area.removeFromBottom(juce::jmin(desired, juce::jmax(50, area.getHeight() / 2)));
        auto input = composerCard.reduced(8, 6); auto controls = input.removeFromBottom(26);
        send->setBounds(controls.removeFromRight(28)); stop->setBounds(send->getBounds()); controls.removeFromRight(6);
        int available = juce::jmax(0, controls.getWidth() - 12);
        int accessWidth = juce::jmin(112, available * 30 / 100);
        int effortWidth = juce::jmin(100, available * 28 / 100);
        access->setBounds(controls.removeFromRight(accessWidth).reduced(0, 2)); controls.removeFromRight(6);
        effort->setBounds(controls.removeFromRight(effortWidth).reduced(0, 2)); controls.removeFromRight(6);
        model->setBounds(controls.reduced(0, 2));
        signIn->setBounds(composerCard.reduced(8).removeFromBottom(26).withWidth(juce::jmin(184, composerCard.getWidth() - 16)));
        reconnectButton.setBounds(signIn->getBounds());
        composer->setBounds(input.withTrimmedBottom(4));
        statusLine = area.removeFromBottom(24); usageButton.setBounds(statusLine.withTrimmedLeft(juce::jmax(0, statusLine.getWidth() - 52)));
        auto label = statusLine.withTrimmedLeft(12); if (signedIn) label.removeFromRight(52);
        statusLabel.setBounds(label); area.removeFromBottom(4);
    }
    viewport.setBounds(area);
    int width = juce::jmax(60, viewport.getWidth() - viewport.getScrollBarThickness());
    if (conversation->getWidth() != width) conversation->setSize(width, conversation->getHeight());
    if (!showingHistory && follow) viewport.setViewPosition(0, juce::jmax(0, conversation->getHeight() - viewport.getHeight()));
    history->setSize(width, history->getHeight());
    emptyState = area.withSizeKeepingCentre(juce::jmin(520, area.getWidth()), juce::jmin(210, area.getHeight()));
    auto chips = emptyState.withTrimmedTop(86);
    for (auto* suggestion : suggestions) suggestion->setBounds(chips.removeFromTop(28).withSizeKeepingCentre(juce::jmin(180, chips.getWidth()), 24));
}
}
