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
}

UndoMaster::~UndoMaster() 
{
}

void UndoMaster::clearUndoHistory()
{
	if (isClearingHistory) return;
	const juce::ScopedValueSetter<bool> clearing(isClearingHistory, true);
	juce::UndoManager::clearUndoHistory();
}

void UndoMaster::performAction(const String & name, UndoableAction * action)
{
	if (action == nullptr) return;
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		delete action;
		return;
	}
	{
		const juce::ScopedValueSetter<bool> performing(isPerforming, true);
		if (transactionGroupDepth == 0) beginNewTransaction(name);
		perform(action, transactionGroupDepth == 0 ? name : transactionGroupName);
	}
	if (Engine::mainEngine != nullptr) Engine::mainEngine->changed();
}

void UndoMaster::performActions(const String & name, Array<UndoableAction*> actions)
{
	actions.removeAllInstancesOf(nullptr);
	if (actions.isEmpty()) return;
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		for (auto* action : actions) delete action;
		return;
	}
	{
		const juce::ScopedValueSetter<bool> performing(isPerforming, true);
		if (transactionGroupDepth == 0) beginNewTransaction(name);
		for (auto* action : actions) perform(action, transactionGroupDepth == 0 ? name : transactionGroupName);
	}
	if (Engine::mainEngine != nullptr) Engine::mainEngine->changed();
}

void UndoMaster::beginGroupedTransaction(const String& name)
{
	jassert(MessageManager::getInstance()->isThisTheMessageThread());
	if (transactionGroupDepth++ == 0)
	{
		transactionGroupName = name;
		beginNewTransaction(name);
	}
}

void UndoMaster::endGroupedTransaction()
{
	jassert(transactionGroupDepth > 0);
	if (transactionGroupDepth > 0 && --transactionGroupDepth == 0)
	{
		beginNewTransaction();
		transactionGroupName.clear();
	}
}
