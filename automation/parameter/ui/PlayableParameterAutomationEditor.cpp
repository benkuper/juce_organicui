#include "PlayableParameterAutomationEditor.h"
#include "../parameter/ParameterAutomation.h"

PlayableParameterAutomationEditor::PlayableParameterAutomationEditor(PlayableParameterAutomation * ppa) :
	ppa(ppa)
{
	automationEditor = ppa->automation.getEditor(false);
	modeUI = ppa->mode->createDefaultUI(); 
	lengthUI = ppa->automation.length->createDefaultUI();

	addAndMakeVisible(automationEditor);
	addAndMakeVisible(modeUI);
	addAndMakeVisible(lengthUI);

	setSize(100, 150);
}

PlayableParameterAutomationEditor::~PlayableParameterAutomationEditor()
{

}

void PlayableParameterAutomationEditor::resized()
{
	Rectangle<int> r = getLocalBounds();
	Rectangle<int> f = r.removeFromBottom(14);
	r.removeFromBottom(2);
	automationEditor->setBounds(r);
	modeUI->setBounds(f.removeFromLeft(80));
	f.removeFromLeft(2);
	lengthUI->setBounds(f);
}
