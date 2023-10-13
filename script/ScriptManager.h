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
	juce::String scriptTemplate;

	Script * createItem() override;

	virtual InspectableEditor * getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;

	bool callFunctionOnAllItems(const juce::Identifier &function, juce::var a = juce::var());
	bool callFunctionOnAllItems(const juce::Identifier &function, juce::Array<juce::var> args);
};
