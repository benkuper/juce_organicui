/*
  ==============================================================================

    ScriptEditor.cpp
    Created: 20 Feb 2017 5:04:20pm
    Author:  Ben

  ==============================================================================
*/

ScriptEditor::ScriptEditor(Script * _script, bool isRoot) :
	BaseItemEditor(_script, isRoot),
	script(_script),
	editMode(false)
{
	script->addAsyncScriptListener(this);

	reloadBT.reset(script->reload->createImageUI(AssetManager::getInstance()->reloadImage));
	
	if (!script->filePath->isControllableFeedbackOnly)
	{
		editBT.reset(AssetManager::getInstance()->getEditBT());
		editBT->addListener(this);
		addAndMakeVisible(editBT.get());
	}

	logUI.reset(script->logParam->createToggle());


	paramsEditor.reset(script->scriptParamsContainer->getEditor(false));
	addChildComponent(paramsEditor.get());
	paramsEditor->setVisible(script->scriptParamsContainer->controllables.size() > 0);

	addAndMakeVisible(reloadBT.get());
	addAndMakeVisible(logUI.get());
}

ScriptEditor::~ScriptEditor()
{
	script->removeAsyncScriptListener(this);
}

void ScriptEditor::paint(Graphics & g)
{
	BaseItemEditor::paint(g);

	Colour c = BG_COLOR;
	switch (script->state)
	{
	case Script::SCRIPT_LOADED:
		c = GREEN_COLOR;
		break;
	case Script::SCRIPT_ERROR:
		c = RED_COLOR;
		break;

	case Script::SCRIPT_EMPTY:
    case Script::SCRIPT_CLEAR:
        c = BG_COLOR.brighter().withAlpha(.4f);
		break;
            
    default:
        break;

	}

	g.setColour(c);
	g.fillEllipse(statusBounds.reduced(4).toFloat());
}

void ScriptEditor::resizedInternalHeaderItemInternal(juce::Rectangle<int>& r)
{ 
	statusBounds = r.removeFromRight(r.getHeight()); 
	r.removeFromRight(2);
	logUI->setBounds(r.removeFromRight(40).reduced(2));
	r.removeFromRight(2);
	reloadBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
	r.removeFromRight(2);

	if (editBT != nullptr) editBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
	r.removeFromRight(2);
}


void ScriptEditor::newMessage(const Script::ScriptEvent & e)
{
	switch (e.type)
	{
	case Script::ScriptEvent::STATE_CHANGE:
			if(script->state == Script::SCRIPT_CLEAR && script->state == Script::ScriptState::SCRIPT_LOADED) resetAndBuild();
			resized();
			repaint();
		break;
	}
}

void ScriptEditor::buttonClicked(Button * b)
{
	BaseItemEditor::buttonClicked(b);
	
	if (b == editBT.get())
	{
		if (script->filePath->stringValue().isEmpty())
		{
			script->chooseFileScript(true);
		} 
	}
}



