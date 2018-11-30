/*
  ==============================================================================

    ScriptManager.h
    Created: 20 Feb 2017 5:00:57pm
    Author:  Ben

  ==============================================================================
*/

#ifndef SCRIPTMANAGER_H_INCLUDED
#define SCRIPTMANAGER_H_INCLUDED

class ScriptManager :
	public BaseManager<Script>
{
public:
	ScriptManager(ScriptTarget * parentTarget);
	~ScriptManager();

	ScriptTarget * parentTarget;
	String scriptTemplate;

	Script * createItem() override;

	virtual InspectableEditor * getEditor(bool isRoot) override;

	bool callFunctionOnAllItems(const Identifier &function, var a = var());
	bool callFunctionOnAllItems(const Identifier &function, Array<var> args);
};


#endif  // SCRIPTMANAGER_H_INCLUDED
