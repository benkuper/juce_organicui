AutomationKey2DUI::AutomationKey2DUI(AutomationKey* key) :
	BaseItemMinimalUI(key),
	keyUINotifier(5)
{
	dragAndDropEnabled = true;
	autoHideWhenDragging = false;
	drawEmptyDragIcon = true;
	highlightColor = HIGHLIGHT_COLOR;
	autoDrawContourWhenSelected = false;

	updateHandles();

	setSize(12,12);
}

AutomationKey2DUI::~AutomationKey2DUI()
{
	for (auto& e : item->easings) e->removeInspectableListener(this);
}

void AutomationKey2DUI::paint(Graphics& g)
{
	Colour c = item->isSelected ? highlightColor : NORMAL_COLOR;
	if (isMouseOverOrDragging()) c = c.brighter();
	g.setColour(c.withAlpha(.5f));
	g.fillEllipse(getLocalBounds().reduced(2).toFloat());
	g.setColour(c);
	g.drawEllipse(getLocalBounds().reduced(2).toFloat(), 1);

}

void AutomationKey2DUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemMinimalUI::controllableFeedbackUpdateInternal(c);
	if (c == item->easingType) updateHandles();
}

void AutomationKey2DUI::inspectableSelectionChanged(Inspectable* i)
{
	BaseItemMinimalUI::inspectableSelectionChanged(i);
	keyUINotifier.addMessage(new Key2DUIEvent(Key2DUIEvent::UI_SELECTION_CHANGED, this));

}

void AutomationKey2DUI::updateHandles()
{
	for (auto& e : item->easings) e->removeInspectableListener(this);

	Array<Easing2DHandle*> removedHandles;
	while (handles.size() > 0) removedHandles.add(handles.removeAndReturn(0));
	keyUINotifier.addMessage(new Key2DUIEvent(Key2DUIEvent::HANDLES_REMOVED, this, removedHandles), true);
	for (auto& h : removedHandles) delete h;

	if (item->easingType->getValueDataAsEnum<Easing::Type>() == Easing::BEZIER)
	{
		for (auto& e : item->easings) e->addInspectableListener(this);
		
		Array<Easing2DHandle*> addedHandles;
		for (int i = 0; i < 2; i++)
		{
			Easing2DHandle* h = new Easing2DHandle(item, i == 0);
			handles.add(h);
			addedHandles.add(h);
			keyUINotifier.addMessage(new Key2DUIEvent(Key2DUIEvent::HANDLES_ADDED, this, addedHandles), true);
		}
	}
	
}

AutomationKey2DUI::Easing2DHandle::Easing2DHandle(AutomationKey* key, bool isFirst) :
	key(key),
	isFirst(isFirst)
{

}

AutomationKey2DUI::Easing2DHandle::~Easing2DHandle()
{

}

void AutomationKey2DUI::Easing2DHandle::paint(Graphics& g)
{
	Colour c = HIGHLIGHT_COLOR;
	if (isMouseOverOrDragging()) c = c.brighter();
	g.setColour(c.withAlpha(.5f));
	g.fillEllipse(getLocalBounds().reduced(2).toFloat());
	g.setColour(c);
	g.drawEllipse(getLocalBounds().reduced(2).toFloat(), 1);
}
