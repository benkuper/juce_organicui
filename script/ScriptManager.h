/*
  ==============================================================================

    ScriptManager.h
    Created: 20 Feb 2017 5:00:57pm
    Author:  Ben

  ==============================================================================
*/

#pragma once


class ScriptManager :
	public BaseManager<Script>
{
public:
	ScriptManager(ScriptTarget * parentTarget);
	~ScriptManager();

	ScriptTarget * parentTarget;
	String scriptTemplate;

	Script * createItem() override;

	virtual InspectableEditor * getEditorInternal(bool isRoot) override;

	bool callFunctionOnAllItems(const Identifier &function, var a = var());
	bool callFunctionOnAllItems(const Identifier &function, Array<var> args);
};
