DetectivePanel::DetectivePanel(const String& name) :
	ManagerShapeShifterUI(name, Detective::getInstance())
{
	addExistingItems();
}

DetectivePanel::~DetectivePanel()
{
}

ControllableDetectiveWatcherUI* DetectivePanel::createUIForItem(ControllableDetectiveWatcher* w)
{
	return w->getUI();
}
