
#include "JuceHeader.h"

CustomLoggerUI::CustomLoggerUI(const String& contentName, CustomLogger* l) :
	ShapeShifterContentComponent(contentName),
	logger(l),
	logList(this),
	totalLogRow(0),
	lastUpdateTime(0)
{

	logger->addLogListener(this);
	std::unique_ptr<TableHeaderComponent> thc(new TableHeaderComponent());
	thc->addColumn(juce::translate("Time"), 1, 60);
	thc->addColumn(juce::translate("Source"), 2, 80);
	thc->addColumn(juce::translate("Content"), 3, 400);


	logListComponent.reset(new TableListBox("CustomLogger", &logList));
	logListComponent->setOpaque(true);
	logListComponent->setRowHeight(13);
	logListComponent->setHeaderHeight(20);
	logListComponent->getViewport()->setScrollBarThickness(10);

	logListComponent->setColour(TableListBox::backgroundColourId, findColour(ResizableWindow::backgroundColourId));
	logListComponent->setHeader(std::move(thc));
	addAndMakeVisible(logListComponent.get());

	LOG(l->getWelcomeMessage());
#if USE_FILE_LOGGER
	LOG(juce::translate("please provide logFile for any bug report :\nlogFile in 123").replace("123", l->fileWriter.getFilePath()));
#endif
	clearB.setButtonText(juce::translate("Clear"));
	clearB.addListener(this);
	addAndMakeVisible(clearB);

	copyB.setButtonText(juce::translate("Copy All"));
	copyB.addListener(this);
	addAndMakeVisible(copyB);
	logListComponent->setMouseCursor(MouseCursor::IBeamCursor);

	autoScrollB.setButtonText(juce::translate("Auto Scroll"));
	autoScrollB.setClickingTogglesState(true);
	autoScrollB.setColour(autoScrollB.buttonColourId, NORMAL_COLOR.darker().withAlpha(.5f));
	autoScrollB.setColour(autoScrollB.buttonOnColourId, NORMAL_COLOR);
	autoScrollB.setToggleState(true, dontSendNotification);
	addAndMakeVisible(autoScrollB);

	logListComponent->setMultipleSelectionEnabled(true);
	setInterceptsMouseClicks(true, false);
	addMouseListener(this, true);

	updateTotalLogRow();
	logListComponent->updateContent();

}


CustomLoggerUI::~CustomLoggerUI()
{
	stopTimer();
	//        logListComponent.setModel(nullptr);
	logger->removeLogListener(this);
}

void CustomLoggerUI::lookAndFeelChanged()
{
	logList.refreshFont();

	if (logListComponent != nullptr)
	{
		logListComponent->updateContent();
		logListComponent->repaint();
	}

	repaint();
}

void CustomLoggerUI::resized()
{

	ShapeShifterContentComponent::resized();
	juce::Rectangle<int> area = getLocalBounds().withTop(5);
	auto footer = area.removeFromBottom(30).reduced(5);

	autoScrollB.setBounds(footer.removeFromRight(60).reduced(2));
	clearB.setBounds(footer.removeFromLeft(footer.getWidth() / 2).reduced(2));
	copyB.setBounds(footer.reduced(2));

	logListComponent->setBounds(area);
	bool firstVisible = area.getWidth() > 400;
	logListComponent->getHeader().setColumnVisible(1, firstVisible);
	bool secondVisible = area.getWidth() > 300;
	logListComponent->getHeader().setColumnVisible(2, secondVisible);

	int tw = getWidth();

	if (firstVisible)tw -= logListComponent->getHeader().getColumnWidth(1);

	if (secondVisible)tw -= logListComponent->getHeader().getColumnWidth(2);

	tw -= logListComponent->getViewport()->getScrollBarThickness();
	tw = jmax(tw, 100);
	logListComponent->getHeader().setColumnWidth(3, tw);
}



void CustomLoggerUI::updateTotalLogRow()
{
	totalLogRow = 0;

	GenericScopedLock lock(logger->logElements.getLock());
	for (auto& l : logger->logElements)
	{
		totalLogRow += l->getNumLines();
	}

}
String CustomLoggerUI::getSourceForRow(const int r) const
{
	if (r < 0) return {};

	int count = 0;
	GenericScopedLock lock(logger->logElements.getLock());
	for (auto* el : logger->logElements)
	{
		const int numLines = el->getNumLines();
		if (count + numLines > r) return el->source;
		count += numLines;
	}
	return {};
}
const bool CustomLoggerUI::isPrimaryRow(const int r) const
{
	int count = 0;
	int idx = 0;

	GenericScopedLock lock(logger->logElements.getLock());
	while (count <= r && idx < logger->logElements.size())
	{
		if (count == r)
		{
			return true;
		}
		if (logger->logElements.size() <= idx) return false;

		count += logger->logElements[idx]->getNumLines();
		idx++;

	}

	return false;
}

String CustomLoggerUI::getContentForRow(const int r) const
{
	if (r < 0) return {};

	int count = 0;
	int idx = 0;
	GenericScopedLock lock(logger->logElements.getLock());

	while (idx < logger->logElements.size())
	{

		int nl = logger->logElements.getUnchecked(idx)->getNumLines();

		if (count + nl > r)
		{
			return logger->logElements.getUnchecked(idx)->getLine(r - count);
		}

		count += nl;
		idx++;
	}

	return {};
};

const String  CustomLoggerUI::getTimeStringForRow(const int r) const
{
	if (r < 0) return {};

	int count = 0;
	GenericScopedLock lock(logger->logElements.getLock());
	for (auto* el : logger->logElements)
	{
		const int numLines = el->getNumLines();
		if (count + numLines > r)
			return getLogTimeString(el->time);
		count += numLines;
	}

	return {};
};

Colour CustomLoggerUI::getSeverityColourForRow(const int r) const
{
	if (r >= 0)
	{
		int count = 0;
		GenericScopedLock lock(logger->logElements.getLock());
		for (auto* el : logger->logElements)
		{
			const int numLines = el->getNumLines();
			if (count + numLines <= r)
			{
				count += numLines;
				continue;
			}

			switch (el->severity)
			{
			case LogElement::LOG_NONE: return logNoneColor;
			case LogElement::LOG_DBG: return logDbgColor;
			case LogElement::LOG_WARN: return Colours::orange;
			case LogElement::LOG_ERR: return Colours::red;
			default: return Colours::pink;
			}
		}
	}

	return Colours::pink;
};


void CustomLoggerUI::mouseDown(const MouseEvent& me) {
	auto pos = me.getEventRelativeTo(logListComponent.get());
	auto rowUnderMouse = logListComponent->getRowContainingPosition(pos.x, pos.y);
	logListComponent->selectRow(rowUnderMouse);
	grabKeyboardFocus();

	if (me.mods.isRightButtonDown())
	{
		PopupMenu p;
		p.addItem(1, "Copy this line (Content only)");
		p.addItem(2, "Copy this line (All)");

		Component::SafePointer<CustomLoggerUI> safeThis(this);
		p.showMenuAsync(PopupMenu::Options(), [this, safeThis, rowUnderMouse](int result)
			{
				if (safeThis == nullptr) return;

				switch (result)
				{
				case 1:
					SystemClipboard::copyTextToClipboard(logList.getTextAt(rowUnderMouse, 3));
					break;

				case 2:
					SystemClipboard::copyTextToClipboard(logList.getTextAt(rowUnderMouse, 1) + "\t" + logList.getTextAt(rowUnderMouse, 2) + "\t" + logList.getTextAt(rowUnderMouse, 3));
					break;
				}
			}
		);
	}
};

void CustomLoggerUI::mouseDrag(const MouseEvent& me) {
	auto pos = me.getEventRelativeTo(logListComponent.get());
	auto rowUnderMouse = logListComponent->getRowContainingPosition(pos.x, pos.y);
	auto rowStart = logListComponent->getRowContainingPosition(pos.getMouseDownX(), pos.getMouseDownY());
	logListComponent->selectRangeOfRows(rowStart, rowUnderMouse);

};

MouseCursor  CustomLoggerUI::getMouseCursor() {
	return MouseCursor::IBeamCursor;
}

void CustomLoggerUI::newMessage(const CustomLogger::LogEvent& message)
{
	//LogElement* el = logger->logElements[logger->logElements.size() - 1];
	//totalLogRow += el->getNumLines();
	//bool overFlow = false;

	//coalesce messages
	if (!Timer::isTimerRunning())
	{
		if (GlobalSettings::getInstanceWithoutCreating() != nullptr)
		{
			int ms = 1000 / GlobalSettings::getInstance()->loggerRefreshRate->intValue();
			startTimer(ms);
		}
	}

}

void CustomLoggerUI::clearLogger()
{
	logger->logElements.clear();
	totalLogRow = 0;
	logListComponent->updateContent();
	LOG(juce::translate("Cleared."));
}

void CustomLoggerUI::timerCallback()
{
	stopTimer();

	updateTotalLogRow();

	//DBG("Handle Async Update");
//    auto cTime = Time::getMillisecondCounter();
//    if(cTime - lastUpdateTime < 500 ){
//        triggerAsyncUpdate();
//    }
//    else{
//        lastUpdateTime = cTime;

	logListComponent->updateContent();
	if (autoScrollB.getToggleState()) logListComponent->scrollToEnsureRowIsOnscreen(totalLogRow.get() - 1);
#if USE_CACHED_GLYPH
	logList.cleanUnusedGlyphs();
#endif
	repaint();

	//    }
}


//////////////
// logList

CustomLoggerUI::LogList::LogList(CustomLoggerUI* o) : minRow(0), maxRow(0), owner(o)
{
}

int CustomLoggerUI::LogList::getNumRows()
{

	return owner->totalLogRow.get();
};

void CustomLoggerUI::LogList::paintRowBackground(Graphics& g,
	int rowNumber,
	int width, int height,
	bool isSelected)
{
	Colour c = owner->getSeverityColourForRow(rowNumber).darker(2);// BG_COLOR.brighter(.1f);// (rowNumber);
	if (rowNumber % 2 == 0) c = c.brighter(.05f);

	g.setColour(c);
	g.fillRect(0, 0, width, height);
};


String CustomLoggerUI::LogList::getTextAt(int rowNumber, int columnId) {
	String text;


	switch (columnId)
	{
	case 1:
		text = owner->isPrimaryRow(rowNumber) ? owner->getTimeStringForRow(rowNumber) : "";
		break;

	case 2:
		text = owner->isPrimaryRow(rowNumber) ? owner->getSourceForRow(rowNumber) : "";
		break;

	case 3:
		text = owner->getContentForRow(rowNumber);
		break;
	}
	return text;

}
#if LOGGER_USE_LABEL
Component* CustomLoggerUI::LogList::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
	Component* existingComponentToUpdate) {
	Colour color = owner->findColour(Label::textColourId);
	String text = getTextAt(rowNumber, columnId);
	Label* lp = nullptr;

	if (existingComponentToUpdate) {
		lp = dynamic_cast<Label*>(existingComponentToUpdate);

	}
	else {
		lp = new Label();
		lp->setFont(logFont);
		lp->setEditable(true);
		//        lp->showEditor();

	}
	jassert(lp);
	if (lp) {
		lp->setText(text, dontSendNotification);
	}
	return lp;
}
#endif
void CustomLoggerUI::LogList::paintCell(Graphics& g,
	int rowNumber,
	int columnId,
	int width, int height,
	bool)
{

	g.setColour(owner->getSeverityColourForRow(rowNumber));


#if !LOGGER_USE_LABEL
	String text = getTextAt(rowNumber, columnId);

#if USE_CACHED_GLYPH
	if (cachedG.contains(text)) {
		auto& cg = cachedG.getReference(text);
		cg.paint(g);
		return;
	}


	auto& cg = cachedG.getReference(text);
	cg.setFont(logFont);
	cg.setText(text);
	cg.setSize(width, height);
	cg.paint(g);
#else
	g.setFont(logFont);
	g.drawFittedText(text, 0, 0, width, height, Justification::left, 1);
#endif

#endif
};

void CustomLoggerUI::LogList::refreshFont()
{
	logFont = Font(12.0f);
}

String CustomLoggerUI::LogList::getCellTooltip(int rowNumber, int /*columnId*/)
{
	const String source = owner->getSourceForRow(rowNumber);
	const String time = owner->getTimeStringForRow(rowNumber);
	const String content = owner->getContentForRow(rowNumber);
	if (source.isEmpty() && time.isEmpty() && content.isEmpty()) return "[Error]";

	return (source.isNotEmpty() ? source + " (" + time + ")\n" : "") + content;
};

#if USE_CACHED_GLYPH
void CustomLoggerUI::LogList::cleanUnusedGlyphs() {
	int nminRow = owner->logListComponent->getRowContainingPosition(1, 1);
	int nmaxRow = owner->logListComponent->getRowContainingPosition(1, owner->logListComponent->getHeight());
	if (nminRow == -1)return;
	if (nmaxRow == -1)nmaxRow = owner->totalLogRow.get();

	int min = 0, max = 0;
	if (nminRow > minRow) {
		min = minRow; max = nminRow;
	}
	if (nmaxRow < maxRow) {
		min = nmaxRow; max = maxRow;
	}

	for (int i = min; i < max; ++i) {
		cachedG.remove(owner->getContentForRow(i));
		if (owner->isPrimaryRow(i)) {
			cachedG.remove(owner->getSourceForRow(i));
			cachedG.remove(owner->getTimeStringForRow(i));
		}
	}

	minRow = nminRow;
	maxRow = nmaxRow;


}
#endif

void CustomLoggerUI::buttonClicked(Button* b)
{

	if (b == &clearB)
	{
		clearLogger();
	}

	else if (b == &copyB) {
		String s;
		for (auto& el : logger->logElements) {
			int leftS = el->source.length() + 3;
			s += el->time.toString(false, true, true, true) + "\t" + el->source + "\t";
			for (int i = 0; i < el->getNumLines(); ++i) {
				if (i != 0)for (int j = 0; j < leftS; j++) s += " ";
				s += el->getLine(i) + "\n";
			}
		}
		SystemClipboard::copyTextToClipboard(s);
	}
}
