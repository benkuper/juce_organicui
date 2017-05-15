#include "ScriptManager.h"
/*
  ==============================================================================

    ScriptManager.cpp
    Created: 20 Feb 2017 5:00:57pm
    Author:  Ben

  ==============================================================================
*/

ScriptManager::ScriptManager(ScriptTarget * _parentTarget):
	BaseManager("Scripts"),
	parentTarget(_parentTarget)
{
	selectItemWhenCreated = false;
}

ScriptManager::~ScriptManager()
{

}

Script * ScriptManager::createItem()
{
	return new Script(parentTarget);
}

InspectableEditor * ScriptManager::getEditor(bool isRoot)
{
	BaseManagerUI<ScriptManager, Script, ScriptUI> * ui = new BaseManagerUI<ScriptManager, Script, ScriptUI>(niceName, this, false);
	ui->drawContour = true; 
	ui->addExistingItems();
	return new GenericComponentEditor(this,ui,isRoot);
}

bool ScriptManager::callFunctionOnAllItems(const Identifier & function, var a)
{
	Array<var> args;
	args.add(a);
	return callFunctionOnAllItems(function, args);
}

bool ScriptManager::callFunctionOnAllItems(const Identifier & function, Array<var> args)
{
	bool result = true;
	for (auto &i : items) result &= i->callFunction(function, args);
	return result;
}
