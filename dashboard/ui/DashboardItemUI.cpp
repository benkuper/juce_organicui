#include "JuceHeader.h"

DashboardItemUI::DashboardItemUI(DashboardItem* item) :
	ItemMinimalUI(item),
	resizer()
{
	bgColor = Colours::transparentBlack;

	autoHideWhenDragging = false;
	drawEmptyDragIcon = true;

	setRepaintsOnMouseActivity(true);
	autoDrawContourWhenSelected = false;

	//resizer.setBorderThickness(BorderSize<int>(6));
	addChildComponent(resizer);
	setInterceptsMouseClicks(true, true);

	setWantsKeyboardFocus(true);

	updateEditMode();

	DashboardManager::getInstance()->editMode->addAsyncParameterListener(this);

	setSize(item->viewUISize->x, item->viewUISize->y);

}

DashboardItemUI::~DashboardItemUI()
{
	if (DashboardManager::getInstanceWithoutCreating() != nullptr) DashboardManager::getInstance()->editMode->removeAsyncParameterListener(this);
}

void DashboardItemUI::paint(Graphics& g)
{
	ItemMinimalUI::paint(g);
}

void DashboardItemUI::paintOverChildren(Graphics& g)
{
	ItemMinimalUI::paintOverChildren(g);
	if (resizer.isVisible() && isMouseOverOrDragging(true))
	{
		g.setColour(Colours::white.withAlpha(.05f));
		g.fillAll();

		Rectangle<int> r = getLocalBounds();
		Line<float> l1(r.getX(), r.getY(), r.getRight(), r.getY());
		Line<float> l2(r.getRight(), r.getY(), r.getRight(), r.getBottom());
		Line<float> l3(r.getRight(), r.getBottom(), r.getX(), r.getBottom());
		Line<float> l4(r.getX(), r.getBottom(), r.getX(), r.getY());
		float dl[] = { 4,2 };

		g.setColour(Colours::white);
		g.drawDashedLine(l1, dl, 2, 1);
		g.drawDashedLine(l2, dl, 2, 1);
		g.drawDashedLine(l3, dl, 2, 1);
		g.drawDashedLine(l4, dl, 2, 1);
	}
}

void DashboardItemUI::resized()
{
	if (resizer.isVisible())
	{
		resizer.setBounds(getLocalBounds().removeFromRight(10).removeFromBottom(10));
	}

	resizedDashboardItemInternal();
}

void DashboardItemUI::mouseDown(const MouseEvent& e)
{
	bool editMode = DashboardManager::getInstance()->editMode->boolValue();
	if (!editMode) return; //avoid dashboard item select when in play mode

	if (e.eventComponent == &resizer)
	{
		this->baseItem->setSizeReference(true);
	}

	ItemMinimalUI::mouseDown(e);
}

void DashboardItemUI::mouseEnter(const MouseEvent& e)
{
	ItemMinimalUI::mouseEnter(e);
	repaint();
}

void DashboardItemUI::mouseDrag(const MouseEvent& e)
{
	bool editMode = DashboardManager::getInstance()->editMode->boolValue();
	if (!editMode) return; //avoid dashboard item select when in play mode
	
	if (e.eventComponent == &resizer)
	{
		ItemMinimalUIListeners.call(&ItemMinimalUIListener::itemUIResizeDrag, this, e.getOffsetFromDragStart());
	}
	else
	{
		ItemMinimalUI::mouseDrag(e);
	}
}

void DashboardItemUI::mouseUp(const MouseEvent& e)
{
	ItemMinimalUI::mouseUp(e);
	
	bool editMode = DashboardManager::getInstance()->editMode->boolValue();
	if (!editMode) return; //avoid dashboard item select when in play mode
	
	if (e.originalComponent == &resizer)
	{
		ItemMinimalUIListeners.call(&ItemMinimalUIListener::itemUIResizeEnd, this);
		//ItemMinimalUIListeners.call(&ItemMinimalUIListener::askForSyncPosAndSize, this);
	}
}

bool DashboardItemUI::keyPressed(const KeyPress& e)
{
	Point<float> posOffset;
	if (e.getKeyCode() == KeyPress::rightKey) posOffset.setX(1);
	else if (e.getKeyCode() == KeyPress::leftKey) posOffset.setX(-1);
	else if (e.getKeyCode() == KeyPress::downKey) posOffset.setY(1);
	else if (e.getKeyCode() == KeyPress::upKey) posOffset.setY(-1);

	if (!posOffset.isOrigin())
	{
		if (e.getModifiers().isShiftDown()) posOffset *= 10;
		else if (e.getModifiers().isAltDown()) posOffset *= 1;
		else  posOffset *= 2;

		item->viewUIPosition->setUndoablePoint(item->viewUIPosition->getPoint() + posOffset, false, false);
		return true;
	}

	return ItemMinimalUI::keyPressed(e);
}

void DashboardItemUI::updateEditMode()
{
	if (DashboardManager::getInstanceWithoutCreating() == nullptr) return;

	bool editMode = DashboardManager::getInstance()->editMode->boolValue();
	updateEditModeInternal(editMode);

	autoSelectWithChildRespect = editMode;
	//item->isSelectable = editMode; //should this be here ?

	autoDrawContourWhenSelected = editMode;
	dragAndDropEnabled = editMode;
	resizer.setVisible(editMode);
	resizer.toFront(false);

	resized();
}

void DashboardItemUI::newMessage(const Parameter::ParameterEvent& e)
{
	if (DashboardManager::getInstanceWithoutCreating() == nullptr) return;

	if (e.parameter == DashboardManager::getInstance()->editMode)
	{
		updateEditMode();
	}

}
