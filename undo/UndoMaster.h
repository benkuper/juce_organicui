/*
  ==============================================================================

    UndoMaster.h
    Created: 12 Apr 2017 8:10:30am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class UndoMaster :
	public UndoManager
{
public:
	juce_DeclareSingleton(UndoMaster,true);
	UndoMaster();
	~UndoMaster();

	bool isPerforming;

	void performAction(const String &name, UndoableAction *action);
	void performActions(const String &name, Array<UndoableAction *> actions);
};
