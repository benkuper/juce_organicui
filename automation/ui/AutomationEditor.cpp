/*
  ==============================================================================

    AutomationEditor.cpp
    Created: 5 Feb 2017 2:58:27pm
    Author:  Ben

  ==============================================================================
*/

AutomationEditor::AutomationEditor(Automation * automation, bool isRoot) :
	GenericControllableContainerEditor(automation, isRoot),
	automation(automation),
	automationUI(nullptr)
{
	if (automation->showUIInEditor)
	{
		automationUI = new AutomationUI(automation);
		automationUI->bgColor = BG_COLOR;
		automationUI->transparentBG = false;
		automationUI->autoResetViewRangeOnLengthUpdate = true;

		addAndMakeVisible(automationUI);
		automationUI->setViewRange(0, automation->length->floatValue());
		setSize(100, 100);
	}
}

AutomationEditor::~AutomationEditor()
{
}

void AutomationEditor::resizedInternalContent(juce::Rectangle<int> &r)
{
	if (automationUI != nullptr)
	{
		if (automationUI != nullptr) automationUI->setBounds(r);
		r.translate(0, automationUI->getHeight());
	}
	
	r.setHeight(0);
}
