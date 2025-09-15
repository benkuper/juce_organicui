#include "DetectivePanel.h"

DetectivePanel::DetectivePanel(const String& name) :
	ManagerShapeShifterUI(name, Detective::getInstance())
{
	addExistingItems();
}

DetectivePanel::~DetectivePanel()
{
}