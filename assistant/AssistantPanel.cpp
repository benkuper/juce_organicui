#include "AssistantPanel.h"
namespace organic::assistant
{
namespace
{
String actionLabel(const String& name)
{
    if (name == "assistant_task") return "Checked requested work";
    if (name == "assistant_progress") return "Updated current activity";
    if (name == "chataigne_project") return "Inspected project";
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
        if (code) { output.append(line, font, colour); continue; }
        auto spans = messageSpans(line); String plain;
        for (auto span : spans) plain += span.text;
        bool bold = false, mono = false; int position = 0;
        for (auto span : spans)
        {
            auto ink = span.tone == MessageTone::error ? RED_COLOR.brighter(.5f) : span.tone == MessageTone::warning ? YELLOW_COLOR.withAlpha(.9f) : colour;
            auto emit = [&](const String& part)
            { output.append(part, mono ? juce::Font(juce::Font::getDefaultMonospacedFontName(), size - 1, juce::Font::plain) : bold ? font.boldened() : font, ink); };
            int start = 0;
            for (int i = 0; i < span.text.length();)
            {
                bool boldMarker = !mono && span.text.substring(i).startsWith("**") && (bold || plain.indexOf(position + i + 2, "**") >= 0);
                bool monoMarker = span.text[i] == '`' && (mono || plain.indexOfChar(position + i + 1, '`') >= 0);
                if (!boldMarker && !monoMarker) { ++i; continue; }
                emit(span.text.substring(start, i));
                if (boldMarker) { bold = !bold; i += 2; } else { mono = !mono; ++i; }
                start = i;
            }
            emit(span.text.substring(start)); position += span.text.length();
        }
    }
    output.setLineSpacing(3.f);
}
}
class Panel::IconButton : public juce::Button
{
public:
    enum Kind { history, settings, send, stop, plus, trash, back, copy, menu };
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
        else if (kind == menu)
        { path.startNewSubPath(4, 6); path.lineTo(8, 10); path.lineTo(12, 6); }
        else
        { path.startNewSubPath(2, 4); path.lineTo(14, 4); path.addRectangle(4, 4, 8, 10); path.startNewSubPath(6, 2); path.lineTo(10, 2); }
        g.strokePath(path, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
private:
    Kind kind;
};
class Panel::ActionButton : public juce::Button
{
public:
    ActionButton() : juce::Button("Send") {}
    void update(bool stop, bool queue)
    {
        stopping = stop;
        auto label = stop ? String("Stop") : queue ? String("Queue") : String("Send");
        setName(label); setTitle(label); setButtonText(label); repaint();
    }
    void paintButton(juce::Graphics& g, bool hover, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced(1);
        auto colour = stopping ? RED_COLOR : HIGHLIGHT_COLOR;
        g.setColour(colour.withAlpha(isEnabled() ? (down ? .85f : hover ? .7f : .55f) : .16f)); g.fillRoundedRectangle(r, 5);
        g.setColour(TEXT_COLOR.withAlpha(isEnabled() ? 1.f : .35f));
        auto icon = r.removeFromLeft(28).withSizeKeepingCentre(12, 12);
        if (stopping) g.fillRoundedRectangle(icon.reduced(1.5f), 1.5f);
        else
        {
            juce::Path arrow; auto c = icon.getCentre();
            arrow.startNewSubPath(c.x, c.y + 5); arrow.lineTo(c.x, c.y - 5);
            arrow.startNewSubPath(c.x - 4, c.y - 1); arrow.lineTo(c.x, c.y - 5); arrow.lineTo(c.x + 4, c.y - 1);
            g.strokePath(arrow, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setFont(juce::Font(12.f, juce::Font::bold)); g.drawText(getButtonText(), r.withTrimmedRight(8), juce::Justification::centredLeft);
    }
private:
    bool stopping = false;
};
class Panel::Conversation : public juce::Component
{
public:
    struct Entry
    {
        String role, text, detail, plainText;
        int count = 0, errors = 0, warnings = 0;
        MessageTone tone = MessageTone::normal;
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
            { Entry entry {"STATUS", line.substring(9).upToFirstOccurrenceOf("]", false, false) + ": " + line.fromFirstOccurrenceOf("]", false, false).trim(), {}};
              entry.tone = line.startsWith("[Status: Failed]") ? MessageTone::error : line.startsWith("[Status: Attention]") ? MessageTone::warning : MessageTone::normal; parsed.push_back(std::move(entry)); }
            else if (line.startsWith("[Tool: "))
            {
                auto name = line.substring(7).upToFirstOccurrenceOf("]", false, false);
                Entry entry {"TOOL", actionLabel(name), {}}; entry.count = 1;
                auto detail = line.fromFirstOccurrenceOf("]", false, false).trim();
                try
                {
                    auto result = mcp::parseBounded(detail);
                    bool failed = result.hasProperty("error") || (result.hasProperty("success") && !(bool) result["success"]);
                    bool recoverable = (bool) result["recoverable"] || (int) result["error"] == -32005 || (int) result["error"] == -32006;
                    entry.errors = failed && !recoverable; entry.warnings = failed && recoverable;
                    if (failed)
                    {
                        auto message = result["message"].toString().isEmpty() ? String("Action failed") : result["message"].toString();
                        entry.detail = errorFragment(message);
                        if (recoverable) entry.detail = entry.detail.replace("<error>", "<warning>").replace("</error>", "</warning>");
                    }
                    else if (name == "assistant_progress") entry.detail = result["title"].toString();
                    else if (!result["name"].toString().isEmpty()) entry.detail = result["name"].toString();
                    else if ((int) result["completedCount"] > 0) entry.detail = String((int) result["completedCount"]) + " operations completed";
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
                { auto& group = next.back(); group.count += entry.count; group.errors += entry.errors; group.warnings += entry.warnings; group.detail += "\n" + detail; }
                else { entry.detail = detail; next.push_back(std::move(entry)); }
            }
            else next.push_back(std::move(entry));
        }
        for (size_t i = 0; i < next.size(); ++i)
        {
            auto& entry = next[i];
            if (entry.role == "TOOL") entry.text = String(entry.count) + (entry.count == 1 ? " project action" : " project actions")
                + (entry.errors > 0 ? " - <warning>" + String(entry.errors) + (entry.errors == 1 ? " needs attention</warning>" : " need attention</warning>")
                    : entry.warnings > 0 ? " - <warning>" + String(entry.warnings) + " retries</warning>" : String());
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
            appendMarkdown(text, tool ? (entry.expanded ? entry.detail : entry.text) : entry.text.trimEnd(), tool ? 11.5f : size, tool ? TEXTNAME_COLOR : TEXT_COLOR);
            if (tool) { juce::AttributedString full; appendMarkdown(full, entry.detail, 11.5f, TEXTNAME_COLOR); entry.plainText = full.getText(); }
            else entry.plainText = text.getText();
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
                auto textY = entry.expanded ? r.getY() + 8 : r.getCentreY() - entry.layout.getHeight() / 2;
                auto centreY = textY + entry.layout.getHeight() / 2;
                if (entry.layout.getNumLines() > 0) { auto lineY = entry.layout.getLine(0).getLineBoundsY(); centreY = textY + lineY.getStart() + lineY.getLength() / 2; }
                auto centreX = r.getX() + 6;
                juce::Path chevron;
                if (entry.expanded) { chevron.startNewSubPath(centreX - 3, centreY - 1.5f); chevron.lineTo(centreX, centreY + 1.5f); chevron.lineTo(centreX + 3, centreY - 1.5f); }
                else { chevron.startNewSubPath(centreX - 1.5f, centreY - 3); chevron.lineTo(centreX + 1.5f, centreY); chevron.lineTo(centreX - 1.5f, centreY + 3); }
                g.setColour(TEXTNAME_COLOR); g.strokePath(chevron, juce::PathStrokeType(1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                entry.layout.draw(g, juce::Rectangle<float>(r.getX() + 18, textY, r.getWidth() - 18, entry.layout.getHeight()));
                continue;
            }
            if (entry.role == "YOU") { g.setColour(PANEL_COLOR.withAlpha(.55f)); g.fillRoundedRectangle(r, 6.f); }
            g.setColour(entry.role == "YOU" ? TEXTNAME_COLOR : entry.tone == MessageTone::warning ? YELLOW_COLOR : entry.tone == MessageTone::error ? RED_COLOR.brighter(.5f) : FEEDBACK_COLOR); g.setFont(juce::Font(10.5f, juce::Font::bold));
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
class Panel::CurrentActivity : public juce::Component, private juce::Timer
{
public:
    CurrentActivity()
    {
        setName("Assistant current activity");
        title.setName("Assistant current activity title"); title.setFont(juce::Font(13.f, juce::Font::bold)); title.setMinimumHorizontalScale(.9f);
        title.setColour(juce::Label::textColourId, TEXT_COLOR);
        detail.setName("Assistant current activity detail"); detail.setFont(10.5f); detail.setColour(juce::Label::textColourId, TEXTNAME_COLOR);
        for (auto* label : {&title, &detail}) { label->setBorderSize(juce::BorderSize<int>(0)); addAndMakeVisible(label); }
    }
    void update(const View& view)
    {
        setVisible(view.busy);
        auto heading = view.turnActive && view.workTitle.isNotEmpty() ? view.workTitle : view.status.upToFirstOccurrenceOf("\n", false, false);
        auto description = view.workDetail;
        if (description.isEmpty())
        {
            if (view.activity == Activity::thinking) description = "Thinking through the next step";
            else if (view.activity == Activity::responding) description = "Writing a response";
            else if (view.activity == Activity::working) description = "Applying project operations";
            else if (view.activity == Activity::connecting) description = "Waiting for the provider connection";
        }
        if (view.turnActive && view.tasks.size() > 0)
        {
            int verified = 0; for (auto task : *view.tasks.getArray()) verified += task["status"].toString() == "completed";
            description += (description.isEmpty() ? String() : " - ") + String(verified) + "/" + String(view.tasks.size()) + " outcomes verified";
        }
        title.setText(heading, juce::dontSendNotification); title.setTooltip(heading);
        detail.setText(description, juce::dontSendNotification); detail.setTooltip(description);
        // Only this small component animates; transcript parsing stays at 5 Hz.
        if (view.busy && isShowing()) { if (!isTimerRunning()) startTimerHz(24); }
        else stopTimer();
    }
    void resized() override
    {
        auto area = getLocalBounds().reduced(12, 8).withTrimmedLeft(28);
        title.setBounds(area.removeFromTop(28)); detail.setBounds(area);
    }
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(.5f);
        g.setColour(HIGHLIGHT_COLOR.withAlpha(.055f)); g.fillRoundedRectangle(bounds, 7);
        g.setColour(HIGHLIGHT_COLOR.withAlpha(.2f)); g.drawRoundedRectangle(bounds, 7, 1);
        auto ring = juce::Rectangle<float>(14, (float) getHeight() / 2 - 8, 16, 16);
        g.setColour(HIGHLIGHT_COLOR.withAlpha(.15f)); g.drawEllipse(ring, 1.7f);
        auto angle = (float) std::fmod(juce::Time::getMillisecondCounterHiRes() / 1200.0, 1.0) * juce::MathConstants<float>::twoPi;
        juce::Path arc; arc.addCentredArc(ring.getCentreX(), ring.getCentreY(), 8, 8, 0, angle, angle + juce::MathConstants<float>::pi * 1.3f, true);
        g.setColour(HIGHLIGHT_COLOR.brighter(.3f)); g.strokePath(arc, juce::PathStrokeType(1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
private:
    juce::Label title, detail;
    void timerCallback() override { if (!isShowing()) stopTimer(); else repaint(); }
};
class Panel::Pending : public juce::Component, public juce::SettableTooltipClient
{
public:
    var rows = mcp::array({});
    bool paused = false;
    std::function<void(const String&)> remove, edit, steer;
    void update(const View& view)
    {
        paused = view.queuePaused;
        if (juce::JSON::toString(rows, true) != juce::JSON::toString(view.pendingMessages, true))
        {
            rows = view.pendingMessages.clone(); removeButtons.clear(); editButtons.clear(); steerButtons.clear();
            for (auto row : *rows.getArray())
            {
                auto id = row["id"].toString();
                auto* removeButton = removeButtons.add(new juce::TextButton("Remove"));
                removeButton->setTooltip("Remove this pending message"); removeButton->onClick = [this, id] { if (remove) remove(id); }; addAndMakeVisible(removeButton);
                auto* editButton = editButtons.add(new juce::TextButton("Edit"));
                editButton->setTooltip("Move this pending message back to the composer\n\n" + row["text"].toString()); editButton->onClick = [this, id] { if (edit) edit(id); }; addAndMakeVisible(editButton);
                auto* steerButton = steerButtons.add(new juce::TextButton("Steer"));
                steerButton->setTooltip("Apply this queued message to the active request now"); steerButton->onClick = [this, id] { if (steer) steer(id); }; addAndMakeVisible(steerButton);
            }
            resized();
        }
        for (int i = 0; i < rows.size(); ++i) steerButtons[i]->setEnabled(view.turnActive && rows[i]["kind"].toString() == "queue");
        repaint();
    }
    void resized() override
    {
        auto height = 24 + rows.size() * rowHeight();
        if (getHeight() != height) setSize(getWidth(), height);
        for (int i = 0; i < rows.size(); ++i)
        {
            auto area = juce::Rectangle<int>(0, 24 + i * rowHeight() + (rowHeight() > 30 ? 22 : 0), getWidth(), 28);
            removeButtons[i]->setBounds(area.removeFromRight(58).reduced(1, 2)); editButtons[i]->setBounds(area.removeFromRight(40).reduced(1, 2));
            steerButtons[i]->setBounds(area.removeFromRight(48).reduced(1, 2));
        }
    }
    void paint(juce::Graphics& g) override
    {
        g.setFont(11.f); g.setColour(TEXTNAME_COLOR);
        g.drawText((paused ? String("Queue paused") : String("Pending messages")) + " (" + String(rows.size()) + ")", 4, 0, getWidth() - 8, 22, juce::Justification::left);
        for (int i = 0; i < rows.size(); ++i)
        {
            auto row = rows[i]; auto text = (row["kind"].toString() == "steer" ? String("Steer at next step: ") : String(i + 1) + ". ") + row["text"].toString().replace("\n", " ");
            g.setColour(TEXT_COLOR); g.drawText(text, 4, 24 + i * rowHeight(), juce::jmax(0, getWidth() - (rowHeight() > 30 ? 8 : 152)), rowHeight() > 30 ? 22 : 28, juce::Justification::left, true);
        }
    }
    void mouseMove(const juce::MouseEvent& event) override
    { auto index = (event.y - 24) / rowHeight(); setTooltip(event.y >= 24 && index < rows.size() ? rows[index]["text"].toString() : String()); }
    void mouseExit(const juce::MouseEvent&) override { setTooltip({}); }
private:
    juce::OwnedArray<juce::TextButton> removeButtons, editButtons, steerButtons;
    int rowHeight() const { return getWidth() < 460 ? 52 : 30; }
};
Panel::Panel(const String& name, Controller& controller) : ShapeShifterContentComponent(name), controller(controller), owner(&controller)
{
    setOpaque(true); conversation = std::make_unique<Conversation>(); history = std::make_unique<History>();
    currentActivity = std::make_unique<CurrentActivity>(); addChildComponent(currentActivity.get());
    history->setName("Conversation history list");
    history->select = [this](const String& id) { if (owner != nullptr) { this->controller.selectConversation(id); showHistory(false); } };
    addAndMakeVisible(viewport); viewport.setViewedComponent(conversation.get(), false); viewport.setScrollBarsShown(true, false); viewport.setScrollBarThickness(7);
    pending = std::make_unique<Pending>(); pending->setName("Pending assistant messages");
    pending->remove = [this](const String& id) { if (owner != nullptr) this->controller.removeQueued(id); };
    pending->edit = [this](const String& id) { if (owner != nullptr) this->controller.editQueued(id); };
    pending->steer = [this](const String& id) { if (owner != nullptr) this->controller.steerQueued(id); };
    addChildComponent(pendingViewport); pendingViewport.setViewedComponent(pending.get(), false); pendingViewport.setScrollBarsShown(true, false); pendingViewport.setScrollBarThickness(7);
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
    primaryAction = std::make_unique<ActionButton>(); addAndMakeVisible(primaryAction.get());
    primaryAction->onClick = [this]
    {
        if (owner == nullptr) return;
        auto view = this->controller.provider.view();
        if (view.busy && (showingHistory || composer->editor.getText().trim().isEmpty() || !view.turnActive)) this->controller.cancelRequest->trigger();
        else this->controller.send->trigger();
        timerCallback();
    };
    actionMenu = icon(IconButton::menu, "Message actions", [this] { showActionMenu(); });
    historyButton = icon(IconButton::history, "Conversation history", [this] { showHistory(!showingHistory); });
    settingsButton = icon(IconButton::settings, "Account and settings", [this] { if (owner != nullptr) this->controller.openSettings->trigger(); });
    newButton = icon(IconButton::plus, "New conversation", [this] { if (owner != nullptr) { this->controller.newConversation->trigger(); showHistory(false); } });
    deleteButton = icon(IconButton::trash, "Delete current conversation", [this] { if (owner != nullptr) this->controller.clearHistory->trigger(); });
    copyButton = icon(IconButton::copy, "Copy conversation", [this]
    { juce::SystemClipboard::copyTextToClipboard(conversationText()); copiedUntil = juce::Time::getMillisecondCounterHiRes() + 2000; timerCallback(); });
    reconnectButton.onClick = [this] { if (owner != nullptr) this->controller.refreshModels->trigger(); };
    reconnectButton.setTooltip("Reconnect to " + controller.provider.info().name + "."); addAndMakeVisible(reconnectButton);
    steerButton.onClick = [this] { if (owner != nullptr) this->controller.steer->trigger(); };
    steerButton.setTooltip("Change the active request before its next project action (Ctrl+Shift+Enter)"); addChildComponent(steerButton);
    resumeButton.onClick = [this] { if (owner != nullptr) this->controller.resumeUnfinished->trigger(); };
    resumeButton.setTooltip("Use any details in the message box, inspect saved changes and finish the remaining work"); addChildComponent(resumeButton);
    runQueueButton.onClick = [this] { if (owner != nullptr) this->controller.runQueue->trigger(); };
    runQueueButton.setTooltip("Start the paused follow-ups without resuming the unfinished request"); addChildComponent(runQueueButton);
    auto button = [this](Trigger* trigger)
    { auto ui = std::unique_ptr<TriggerButtonUI>(trigger->createButtonUI()); ui->customTextSize = 12; addAndMakeVisible(ui.get()); return ui; };
    signIn = button(controller.signIn);
    for (auto* trigger : {controller.explainProject, controller.findIssues, controller.buildWorkflow}) suggestions.add(button(trigger).release());
    composer->editor.addListener(this);
    addKeyListener(this); viewport.addKeyListener(this); timerCallback(); startTimerHz(5);
}
Panel::~Panel() { stopTimer(); removeKeyListener(this); viewport.removeKeyListener(this); composer->editor.removeListener(this); composer->editor.removeKeyListener(this); viewport.setViewedComponent(nullptr, false); pendingViewport.setViewedComponent(nullptr, false); }
String Panel::conversationText() const
{
    if (owner == nullptr) return {};
    auto view = controller.provider.view(); String queued;
    for (auto message : *view.pendingMessages.getArray()) queued += "\n" + message["kind"].toString() + ": " + message["text"].toString();
    return "Chataigne assistant\nProvider: " + controller.provider.info().name + "\nModel: " + controller.models->getValueKey() + "\nEffort: " + controller.effort->getValueKey()
        + "\nAccess: " + controller.access->getValueKey() + "\nStatus: " + controller.status->stringValue()
        + (view.busy && view.workTitle.isNotEmpty() ? "\nCurrent activity: " + view.workTitle + (view.workDetail.isEmpty() ? String() : " - " + view.workDetail) : String())
        + "\n\n" + conversation->copyText()
        + (queued.isEmpty() ? String() : "\n\nPending messages:" + queued);
}
void Panel::showHistory(bool show)
{
    if (show) conversationScroll = viewport.getViewPositionY();
    showingHistory = show; viewport.setViewedComponent(show ? (juce::Component*) history.get() : conversation.get(), false);
    historyButton->setKind(show ? IconButton::back : IconButton::history);
    viewport.setViewPosition(0, show ? 0 : conversationScroll); timerCallback();
}
void Panel::updateActions()
{
    if (owner == nullptr || !primaryAction) return;
    auto view = controller.provider.view(); auto hasText = composer->editor.getText().trim().isNotEmpty();
    bool stop = view.busy && (showingHistory || !hasText || !view.turnActive);
    bool queue = view.turnActive || view.pendingMessages.size() > 0;
    primaryAction->update(stop, queue);
    primaryAction->setTooltip(stop ? "Stop the active request; keep your draft and pause follow-ups" : queue ? "Queue follow-up (Ctrl+Enter)" : "Send (Ctrl+Enter)");
    primaryAction->setEnabled(stop || (controller.send->enabled && hasText));
    primaryAction->setVisible(showingHistory ? view.busy : view.signedIn || view.busy);
    actionMenu->setVisible(view.busy && !showingHistory);
    bool showSteer = !showingHistory && view.turnActive && hasText;
    bool layoutChanged = steerButton.isVisible() != showSteer;
    steerButton.setVisible(showSteer); steerButton.setEnabled(controller.steer->enabled && hasText);
    if (layoutChanged) { resized(); repaint(); }
}
void Panel::showActionMenu()
{
    if (owner == nullptr) return;
    controller.refresh(); auto view = controller.provider.view(); auto hasText = composer->editor.getText().trim().isNotEmpty();
    juce::PopupMenu menu;
    menu.addItem(1, "Queue message", controller.send->enabled && hasText);
    menu.addItem(2, "Steer", view.turnActive && hasText);
    menu.addSeparator(); menu.addItem(3, "Stop request", view.busy);
    juce::Component::SafePointer<Panel> safe(this);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(actionMenu.get()), [safe](int result)
    {
        if (safe == nullptr || safe->owner == nullptr) return;
        if (result == 1) safe->controller.send->trigger();
        else if (result == 2) safe->controller.steer->trigger();
        else if (result == 3) safe->controller.cancelRequest->trigger();
        safe->timerCallback();
    });
}
bool Panel::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (owner == nullptr) return false;
    if (showingHistory && key.getKeyCode() == juce::KeyPress::escapeKey) { showHistory(false); return true; }
    if (key.getKeyCode() == juce::KeyPress::returnKey && (key.getModifiers().isCtrlDown() || key.getModifiers().isCommandDown()))
    { if (key.getModifiers().isShiftDown() && controller.steer->enabled) controller.steer->trigger(); else controller.send->trigger(); return true; } return false;
}
void Panel::timerCallback()
{
    if (owner == nullptr) { stopTimer(); return; }
    controller.refresh(); auto view = controller.provider.view();
    isBusy = view.busy; turnActive = view.turnActive; signedIn = view.signedIn; activity = view.activity; statusText = controller.status->stringValue();
    currentActivity->update(view);
    copyButton->setTooltip(juce::Time::getMillisecondCounterHiRes() < copiedUntil ? "Conversation copied" : "Copy conversation");
    auto transcript = view.transcript;
    if (!controller.currentError().isEmpty()) transcript += "\n[Status: Failed] " + controller.currentError();
    else if (activity == Activity::failed && view.recovery.isEmpty() && statusText != "Some actions need attention" && !transcript.contains(statusText)) transcript += "\n[Status: Failed] " + statusText;
    if (displayedTranscript != transcript || displayedConversation != view.conversation)
    {
        bool switched = displayedConversation != view.conversation;
        bool follow = viewport.getViewPositionY() + viewport.getHeight() >= conversation->getHeight() - 50;
        displayedConversation = view.conversation; displayedTranscript = transcript; conversation->update(displayedTranscript);
        if (!showingHistory && (follow || switched)) viewport.setViewPosition(0, juce::jmax(0, conversation->getHeight() - viewport.getHeight()));
    }
    history->update(view);
    pending->update(view); pendingViewport.setVisible(!showingHistory && view.pendingMessages.size() > 0);
    resumeButton.setVisible(!showingHistory && !isBusy && view.canResume); resumeButton.setEnabled(controller.resumeUnfinished->enabled);
    runQueueButton.setVisible(!showingHistory && !isBusy && view.queuePaused && view.pendingMessages.size() > 0);
    composer->editor.setTextToShowWhenEmpty(turnActive ? "Queue a follow-up, or Steer to change this task..."
        : view.canResume ? "Add any missing details, then Resume unfinished..." : "Ask about your project...", TEXTNAME_COLOR);
    updateActions(); signIn->setVisible(!signedIn && !isBusy && !showingHistory);
    reconnectButton.setVisible(signedIn && view.models.size() == 0 && !isBusy && !showingHistory);
    composer->setVisible(!showingHistory); model->setVisible(signedIn && view.models.size() > 0 && !showingHistory); access->setVisible(signedIn && view.models.size() > 0 && !showingHistory);
    effort->setVisible(signedIn && view.models.size() > 0 && !showingHistory); copyButton->setVisible(!showingHistory);
    newButton->setVisible(showingHistory); deleteButton->setVisible(showingHistory && !displayedTranscript.trim().isEmpty());
    newButton->setEnabled(true); deleteButton->setEnabled(!isBusy);
    historyButton->setTooltip(showingHistory ? "Back to conversation" : "Conversation history");
    for (auto* suggestion : suggestions) suggestion->setVisible(!showingHistory && displayedTranscript.trim().isEmpty() && signedIn);
    resized(); repaint();
}
void Panel::paint(juce::Graphics& g)
{
    g.fillAll(BG_COLOR);
    auto title = header.reduced(12, 0); title.removeFromRight(showingHistory ? (isBusy ? 200 : 124) : 92);
    g.setColour(TEXT_COLOR); g.setFont(juce::Font(12.f, juce::Font::bold));
    g.drawText(showingHistory ? "Conversations" : "Assistant", title, juce::Justification::left);
    g.setColour(CONTOUR_COLOR.withAlpha(.6f)); g.drawHorizontalLine(header.getBottom() - 1, 0.f, (float) getWidth());
    if (showingHistory) return;
    g.setColour(BG_COLOR.brighter(.045f)); g.fillRoundedRectangle(composerCard.toFloat(), 7.f);
    g.setColour(CONTOUR_COLOR.withAlpha(.8f)); g.drawRoundedRectangle(composerCard.toFloat().reduced(.5f), 7.f, 1);
    if (displayedTranscript.trim().isEmpty())
    {
        auto area = emptyState; g.setColour(TEXT_COLOR); g.setFont(juce::Font(17.f, juce::Font::bold));
        g.drawFittedText(signedIn ? "What would you like to build?" : "Your project assistant", area.removeFromTop(42), juce::Justification::centred, 2);
        g.setFont(12.f); g.setColour(TEXTNAME_COLOR);
        g.drawFittedText(signedIn ? "Explore, troubleshoot, or create a workflow." : controller.provider.info().signInHint, area.removeFromTop(38), juce::Justification::centred, 2);
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
    { newButton->setBounds(actions.removeFromRight(28)); deleteButton->setBounds(actions.removeFromRight(28)); if (isBusy) primaryAction->setBounds(actions.removeFromRight(76)); }
    area.reduce(8, 6);
    if (!showingHistory)
    {
        int lines = juce::jmax(1, composer->editor.getText().length() / juce::jmax(16, getWidth() / 7));
        lines = juce::jmax(lines, juce::StringArray::fromLines(composer->editor.getText()).size());
        int actionWidth = (primaryAction->isVisible() ? 84 : 0) + (actionMenu->isVisible() ? 27 : 0) + (steerButton.isVisible() ? 72 : 0);
        bool singleRow = getWidth() - 32 >= actionWidth + 278;
        int desired = signedIn ? juce::jlimit(singleRow ? 78 : 108, singleRow ? 144 : 174, (singleRow ? 56 : 86) + lines * 18) : 92;
        composerCard = area.removeFromBottom(juce::jmin(desired, juce::jmax(50, area.getHeight() / 2)));
        auto input = composerCard.reduced(8, 6); auto controls = input.removeFromBottom(30);
        auto messageActions = singleRow ? controls.removeFromRight(actionWidth) : controls;
        if (actionMenu->isVisible()) { actionMenu->setBounds(messageActions.removeFromRight(24)); messageActions.removeFromRight(3); }
        if (primaryAction->isVisible()) primaryAction->setBounds(messageActions.removeFromRight(84).reduced(0, 1));
        if (steerButton.isVisible()) { messageActions.removeFromRight(6); steerButton.setBounds(messageActions.removeFromRight(66).reduced(0, 2)); }
        if (singleRow) controls.removeFromRight(6);
        else { input.removeFromBottom(4); controls = input.removeFromBottom(26); }
        int available = juce::jmax(0, controls.getWidth() - 12);
        int accessWidth = juce::jmin(112, available * 30 / 100);
        int effortWidth = juce::jmin(100, available * 28 / 100);
        access->setBounds(controls.removeFromRight(accessWidth).reduced(0, 2)); controls.removeFromRight(6);
        effort->setBounds(controls.removeFromRight(effortWidth).reduced(0, 2)); controls.removeFromRight(6);
        model->setBounds(controls.reduced(0, 2));
        signIn->setBounds(composerCard.reduced(8).removeFromBottom(26).withWidth(juce::jmin(184, composerCard.getWidth() - 16)));
        reconnectButton.setBounds(signIn->getBounds());
        composer->setBounds(input.withTrimmedBottom(4));
        area.removeFromBottom(6);
        if (resumeButton.isVisible() || runQueueButton.isVisible())
        { auto recoveryActions = area.removeFromBottom(30); resumeButton.setBounds(recoveryActions.removeFromLeft(136).reduced(0, 2)); runQueueButton.setBounds(recoveryActions.removeFromRight(88).reduced(0, 2)); }
        if (pendingViewport.isVisible())
        { pending->setSize(juce::jmax(60, area.getWidth() - 7), pending->getHeight()); pendingViewport.setBounds(area.removeFromBottom(juce::jmin(166, pending->getHeight()))); area.removeFromBottom(4); }
    }
    if (currentActivity->isVisible())
    { currentActivity->setBounds(area.removeFromBottom(64)); area.removeFromBottom(8); }
    viewport.setBounds(area);
    int width = juce::jmax(60, viewport.getWidth() - viewport.getScrollBarThickness());
    if (conversation->getWidth() != width) conversation->setSize(width, conversation->getHeight());
    if (!showingHistory && follow) viewport.setViewPosition(0, juce::jmax(0, conversation->getHeight() - viewport.getHeight()));
    history->setSize(width, history->getHeight());
    emptyState = area.withSizeKeepingCentre(juce::jmin(520, area.getWidth()), juce::jmin(210, area.getHeight()));
    auto chips = emptyState.withTrimmedTop(86);
    for (auto* suggestion : suggestions) suggestion->setBounds(chips.removeFromTop(28).withSizeKeepingCentre(juce::jmin(180, chips.getWidth()), 24));
}
HubPanel::HubPanel(const String& name, Hub& hub) : ShapeShifterContentComponent(name), hub(hub), owner(&hub)
{
    provider.reset(hub.providers->createUI()); provider->setName("Assistant provider selector"); addAndMakeVisible(provider.get());
    for (auto* controller : hub.controllers) { auto* panel = new Panel(name, *controller); panels.add(panel); addChildComponent(panel); }
    timerCallback(); startTimerHz(5);
}
HubPanel::~HubPanel() { stopTimer(); }
void HubPanel::timerCallback()
{
    if (!owner) { stopTimer(); return; }
    for (int i = 0; i < panels.size(); ++i) panels[i]->setVisible(hub.controllers[i] == &hub.active());
}
void HubPanel::resized()
{
    auto area = getLocalBounds(); provider->setBounds(area.removeFromTop(30).reduced(8, 3));
    for (auto* panel : panels) panel->setBounds(area);
}
}
