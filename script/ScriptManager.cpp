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
	parentTarget(_parentTarget),
	scriptTemplate("generic")
{
	selectItemWhenCreated = false;
}

ScriptManager::~ScriptManager()
{

}

Script * ScriptManager::createItem()
{
	Script * s = new Script(parentTarget);
	s->scriptTemplate = &scriptTemplate;
	return s;
}

InspectableEditor * ScriptManager::getEditor(bool isRoot)
{
	return BaseManager::getEditor(isRoot);
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
	for (auto &i : items)
	{
		Result r = Result::ok();
		i->callFunction(function, args, &r);
		if (r != Result::ok()) result = false;
	}

	return result;
}
