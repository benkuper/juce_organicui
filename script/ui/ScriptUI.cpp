/*
  ==============================================================================

    ScriptUI.cpp
    Created: 20 Feb 2017 5:04:20pm
    Author:  Ben

  ==============================================================================
*/

ScriptUI::ScriptUI(Script * _script) :
	BaseItemUI<Script>(_script),
	script(_script),
	editMode(false)
{
	script->addAsyncScriptListener(this);

	fileBT = AssetManager::getInstance()->getFileBT();
	reloadBT = script->reload->createImageUI(AssetManager::getInstance()->getReloadImage());
	editBT = AssetManager::getInstance()->getEditBT();
	logUI = script->logParam->createToggle();

	fileBT->addListener(this);
	editBT->addListener(this);

	paramsEditor = script->scriptParamsContainer.getEditor(false);
	addChildComponent(paramsEditor);
	paramsEditor->setVisible(script->scriptParamsContainer.controllables.size() > 0);

	addAndMakeVisible(fileBT);
	addAndMakeVisible(reloadBT);
	addAndMakeVisible(editBT);
	addAndMakeVisible(logUI);
}

ScriptUI::~ScriptUI()
{
	script->removeAsyncScriptListener(this);
}

void ScriptUI::paint(Graphics & g)
{
	BaseItemUI::paint(g);
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

	}

	g.setColour(c);
	g.fillEllipse(statusBounds.reduced(2).toFloat());
}

void ScriptUI::resizedInternalHeader(Rectangle<int>& r)
{ 
	DBG("resize internal header"); 
	statusBounds = r.removeFromRight(r.getHeight());
	r.removeFromRight(2);
	logUI->setBounds(r.removeFromRight(40));
	r.removeFromRight(2);
	editBT->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);
	reloadBT->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);
	fileBT->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);
}

void ScriptUI::resizedInternalContent(Rectangle<int>& r)
{
	DBG("resized internal content " << (int)paramsEditor->isVisible() << "/ " << (int)script->state);
	if (script->state != Script::SCRIPT_CLEAR && paramsEditor->isVisible() )
	{
		DBG("Set editor height : " << paramsEditor->getHeight());
		paramsEditor->resized();
		r.setHeight(paramsEditor->getHeight());
		paramsEditor->setBounds(r);
	}
	
	if (!editMode) return;
	//text editor here
}


void ScriptUI::newMessage(const Script::ScriptEvent & e)
{
	switch (e.type)
	{
	case Script::ScriptEvent::STATE_CHANGE:
		if (script->state != Script::SCRIPT_CLEAR)
		{
			repaint();
			paramsEditor->setVisible(script->scriptParamsContainer.controllables.size() > 0);
			resized();
		}

		break;
	}
}

void ScriptUI::buttonClicked(Button * b)
{
	BaseItemUI::buttonClicked(b);
	if (b == fileBT)
	{
		FileChooser chooser("Load a cacahuete");
		bool result = chooser.browseForFileToOpen();
		if (result) script->filePath->setValue(chooser.getResult().getFullPathName());

	} else if (b == editBT)
	{
		if (script->filePath->stringValue().isEmpty())
		{
			FileChooser chooser("Create a new cacahuete",File(),"*.js");
			bool result = chooser.browseForFileToSave(true);
			if (result)
			{
				chooser.getResult().create();
				script->filePath->setValue(chooser.getResult().getFullPathName());
			}
		} 


		
		if (script->filePath->stringValue().isEmpty()) return;

		File f(Engine::mainEngine->getFile().getParentDirectory().getChildFile(script->filePath->stringValue()));
		f.startAsProcess();
		//editMode = !editMode;
		//resized();
	}
}



