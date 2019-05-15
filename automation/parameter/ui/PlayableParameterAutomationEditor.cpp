PlayableParameterAutomationEditor::PlayableParameterAutomationEditor(PlayableParameterAutomation * ppa) :
	ppa(ppa)
{
	automationEditor.reset(ppa->automation.getEditor(false));
	modeUI.reset(ppa->mode->createDefaultUI());
	lengthUI.reset(ppa->automation.length->createDefaultUI());

	addAndMakeVisible(automationEditor.get());
	addAndMakeVisible(modeUI.get());
	addAndMakeVisible(lengthUI.get());

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
