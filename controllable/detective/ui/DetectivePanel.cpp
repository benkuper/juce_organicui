
DetectivePanel::DetectivePanel(const String& name) :
	ShapeShifterContentComponent(name),
	managerEditor(Detective::getInstance(), true)
{
	addAndMakeVisible(&managerEditor);
}

DetectivePanel::~DetectivePanel()
{
}

void DetectivePanel::resized()
{
	managerEditor.setBounds(getLocalBounds());
}
