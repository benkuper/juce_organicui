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

	reloadBT = script->reload->createImageUI(AssetManager::getInstance()->getReloadImage());
	editBT = AssetManager::getInstance()->getEditBT();
	logUI = script->logParam->createToggle();

	editBT->addListener(this);

	paramsEditor = script->scriptParamsContainer.getEditor(false);
	addChildComponent(paramsEditor);
	paramsEditor->setVisible(script->scriptParamsContainer.controllables.size() > 0);

	addAndMakeVisible(reloadBT);
	addAndMakeVisible(editBT);
	addAndMakeVisible(logUI);
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
	g.fillEllipse(statusBounds.reduced(2).toFloat());
}

void ScriptEditor::resizedInternalHeaderItemInternal(juce::Rectangle<int>& r)
{ 
	statusBounds = r.removeFromRight(r.getHeight()); 
	r.removeFromRight(2);
	logUI->setBounds(r.removeFromRight(40).reduced(2));
	r.removeFromRight(2);
	reloadBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
	r.removeFromRight(2);
	editBT->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
	r.removeFromRight(2);
}


void ScriptEditor::newMessage(const Script::ScriptEvent & e)
{
	switch (e.type)
	{
	case Script::ScriptEvent::STATE_CHANGE:
			resetAndBuild();
			resized();
			repaint();
		break;
	}
}

void ScriptEditor::buttonClicked(Button * b)
{
	BaseItemEditor::buttonClicked(b);
	
	if (b == editBT)
	{
		if (script->filePath->stringValue().isEmpty())
		{
			FileChooser chooser("Create a new cacahuete",File(),"*.js");
			bool result = chooser.browseForFileToSave(true);
			if (result)
			{
				File f = chooser.getResult();
				if (!f.exists())
				{
					f.create();

					if (script->scriptTemplate->isNotEmpty())
					{
						FileOutputStream fos(f);
						if (fos.openedOk())
						{
							fos.writeText(*script->scriptTemplate,false,false,"\n");
							fos.flush();
						}
					}
				}

				script->filePath->setValue(chooser.getResult().getFullPathName());
			}
		} 
		
		if (script->filePath->stringValue().isEmpty()) return;

		File f = File();
		if (Engine::mainEngine != nullptr) f = Engine::mainEngine->getFile().getParentDirectory().getChildFile(script->filePath->stringValue());
		else f = File(script->filePath->getAbsolutePath());

		if(f.existsAsFile()) f.startAsProcess();
		
		//editMode = !editMode;
		//resized();
	}
}



