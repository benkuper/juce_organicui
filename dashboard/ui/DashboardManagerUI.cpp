/*
  ==============================================================================

	DashboardManagerUI.cpp
	Created: 19 Apr 2017 11:00:50pm
	Author:  Ben

  ==============================================================================
*/


DashboardManagerUI::DashboardManagerUI(DashboardManager* manager) :
	BaseManagerUI("Dashboards", manager)
{
	setDefaultLayout(Layout::HORIZONTAL);

	bgColor = bgColor.darker();

	editModeUI.reset(manager->editMode->createToggle());
	snappingUI.reset(manager->snapping->createToggle());
	addAndMakeVisible(editModeUI.get());
	addAndMakeVisible(snappingUI.get());
	editModeUI->showLabel = true;
	snappingUI->showLabel = true;

	addExistingItems();
}


DashboardManagerUI::~DashboardManagerUI()
{

}

void DashboardManagerUI::resizedInternalContent(juce::Rectangle<int>& r)
{
	juce::Rectangle<int> br = r.removeFromRight(100).reduced(4);
	editModeUI->setBounds(br.removeFromTop(br.getHeight() / 2).reduced(1));
	snappingUI->setBounds(br.reduced(1));
	BaseManagerUI::resizedInternalContent(r);
}