#include "DetectivePanel.h"

DetectivePanel::DetectivePanel(const String& name) :
	BaseManagerShapeShifterUI(name, Detective::getInstance())
{
}

DetectivePanel::~DetectivePanel()
{
}

ControllableDetectiveWatcherUI* DetectivePanel::createUIForItem(ControllableDetectiveWatcher* w)
{
	return w->getUI();
}
