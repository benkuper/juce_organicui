#include "DashboardItemUI.h"

DashboardItemUI::DashboardItemUI(DashboardItem* item) :
	BaseItemMinimalUI(item),
	resizer(this, nullptr)
{
	bgColor = Colours::transparentBlack;

	autoHideWhenDragging = false;
	drawEmptyDragIcon = true;

	setRepaintsOnMouseActivity(true);
	autoDrawContourWhenSelected = false;

	resizer.setBorderThickness(BorderSize<int>(6));
	addChildComponent(resizer);
	setInterceptsMouseClicks(true, true);

	updateEditMode();

	DashboardManager::getInstance()->editMode->addAsyncParameterListener(this);

	item->viewUISize->addAsyncParameterListener(this);
}

DashboardItemUI::~DashboardItemUI()
{
	if (DashboardManager::getInstanceWithoutCreating() != nullptr) DashboardManager::getInstance()->editMode->removeAsyncParameterListener(this);

}

void DashboardItemUI::paint(Graphics& g)
{
	BaseItemMinimalUI::paint(g);
}

void DashboardItemUI::paintOverChildren(Graphics& g)
{
	BaseItemMinimalUI::paintOverChildren(g);
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
		resizer.setBounds(getLocalBounds());
	}

	resizedDashboardItemInternal();
}

void DashboardItemUI::mouseEnter(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseEnter(e);
	repaint();
}

void DashboardItemUI::mouseUp(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseUp(e);
	if (e.originalComponent == &resizer)
	{
		itemMinimalUIListeners.call(&ItemMinimalUIListener::askForSyncPosAndSize, this);
	}
}

void DashboardItemUI::updateEditMode()
{
	if (DashboardManager::getInstanceWithoutCreating() == nullptr) return;

	bool editMode = DashboardManager::getInstance()->editMode->boolValue();
	updateEditModeInternal(editMode);

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
	else if (e.parameter == item->viewUISize)
	{
		setSize(item->viewUISize->x, item->viewUISize->y);
	}
}
