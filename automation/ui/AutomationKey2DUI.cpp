
AutomationKey2DUI::AutomationKey2DUI(AutomationKey* key) :
	BaseItemMinimalUI(key)
{
	dragAndDropEnabled = true;
	autoHideWhenDragging = false;
	drawEmptyDragIcon = true;
	highlightColor = HIGHLIGHT_COLOR;
	autoDrawContourWhenSelected = false;

	setSize(12,12);
}

AutomationKey2DUI::~AutomationKey2DUI()
{
}

void AutomationKey2DUI::paint(Graphics& g)
{
	Colour c = item->isSelected ? highlightColor : Colours::white;
	if (isMouseOverOrDragging()) c = c.brighter();
	g.setColour(c.withAlpha(.5f));
	g.fillEllipse(getLocalBounds().reduced(2).toFloat());
	g.setColour(c);
	g.drawEllipse(getLocalBounds().reduced(2).toFloat(), 1);

}
