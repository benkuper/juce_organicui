/*
  ==============================================================================

    AutomationEditor.cpp
    Created: 5 Feb 2017 2:58:27pm
    Author:  Ben

  ==============================================================================
*/

AutomationEditor::AutomationEditor(Automation * automation, bool isRoot) :
	EnablingControllableContainerEditor(automation, isRoot),
	automation(automation),
	automationUI(nullptr)
{
	resetAndBuild();
	setSize(100, 100);
}

AutomationEditor::~AutomationEditor()
{
}

void AutomationEditor::resizedInternalContent(juce::Rectangle<int> &r)
{

	if (automationUI != nullptr && !container->editorIsCollapsed)
	{
		if (r.getHeight() == 0) r.setHeight(automationUI->getHeight());
		automationUI->setBounds(r);
		r.translate(0, automationUI->getHeight());
	}
	
	r.setHeight(0);
}

void AutomationEditor::resetAndBuild()
{
	EnablingControllableContainerEditor::resetAndBuild();
	if (container->editorIsCollapsed)
	{
		if (automationUI != nullptr)
		{
			removeChildComponent(automationUI.get());
			automationUI.reset();
		}
	}else
	{
		if (automationUI == nullptr)
		{
			automationUI.reset(new AutomationUI(automation));

			automationUI->bgColor = BG_COLOR;
			automationUI->transparentBG = false;
			automationUI->autoAdaptViewRange = true;
			addAndMakeVisible(automationUI.get());
			automationUI->setViewRange(0, automation->length->floatValue());
		}
	}
}
