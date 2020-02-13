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
		if (automation->numDimensions == 1)
		{
			AutomationUI* aui = new AutomationUI(automation);
			aui->bgColor = BG_COLOR;
			aui->transparentBG = false;
			aui->autoResetViewRangeOnLengthUpdate = true;
			aui->setViewRange(0, automation->length->floatValue());
			automationUI.reset(aui);
		}
		else if (automation->numDimensions == 2)
		{
			automationUI.reset(new Automation2DUI(automation));
			setSize(100, 400);
		}
		
		if (automationUI != nullptr)
		{
			addAndMakeVisible(automationUI.get());
		}
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
