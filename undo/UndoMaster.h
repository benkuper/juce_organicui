/*
  ==============================================================================

    UndoMaster.h
    Created: 12 Apr 2017 8:10:30am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class UndoMaster :
	public juce::UndoManager
{
public:
	juce_DeclareSingleton(UndoMaster,true);
	UndoMaster();
	~UndoMaster();

	bool isPerforming;

	void performAction(const juce::String &name, juce::UndoableAction *action);
	void performActions(const juce::String &name, juce::Array<juce::UndoableAction *> actions);
};
