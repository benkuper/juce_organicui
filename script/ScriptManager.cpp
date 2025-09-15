/*
  ==============================================================================

    ScriptManager.cpp
    Created: 20 Feb 2017 5:00:57pm
    Author:  Ben

  ==============================================================================
*/

ScriptManager::ScriptManager(ScriptTarget * _parentTarget):
	Manager("Scripts"),
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

InspectableEditor * ScriptManager::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new ScriptManagerEditor(this, isRoot);
}

bool ScriptManager::callFunctionOnAllScripts(const Identifier & function, var a)
{
	Array<var> args;
	args.add(a);
	return callFunctionOnAllScripts(function, args);
}

bool ScriptManager::callFunctionOnAllScripts(const Identifier & function, Array<var> args)
{
	bool result = true;
	Array<Script*> items = getItems();
	for (auto &i : items)
	{
		Result r = Result::ok();
		i->callFunction(function, args, &r);
		if (r != Result::ok()) result = false;
	}

	return result;
}
