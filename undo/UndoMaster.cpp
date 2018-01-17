/*
  ==============================================================================

  UndoMaster.cpp
  Created: 12 Apr 2017 8:10:30am
  Author:  Ben

  ==============================================================================
  */

juce_ImplementSingleton(UndoMaster);

UndoMaster::UndoMaster() :
	isPerforming(false)
{
	DBG("Undomaster constructor");
}

UndoMaster::~UndoMaster() 
{
	DBG("Undo master destructor");
}

void UndoMaster::performAction(const String & name, UndoableAction * action)
{
	if (Engine::mainEngine->isLoadingFile) return;
	DBG("Perform Action " << name);
	isPerforming = true;
	beginNewTransaction(name);
	perform(action,name);
	isPerforming = false;
}

void UndoMaster::performActions(const String & name, Array<UndoableAction*> actions)
{
	if (Engine::mainEngine->isLoadingFile) return;
	DBG("Perform Actions " << name);
	isPerforming = true;
	beginNewTransaction(name);
	for (auto &a : actions) perform(a,name);
	isPerforming = false;
}
