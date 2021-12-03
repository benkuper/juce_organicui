/*
  ==============================================================================

	TriggerUI.cpp
	Created: 8 Mar 2016 3:48:52pm
	Author:  bkupe

  ==============================================================================
*/

TriggerUI::TriggerUI(Array<Trigger*> triggers) :
	ControllableUI(Inspectable::getArrayAs<Trigger, Controllable>(triggers)),
	triggers(Inspectable::getWeakArray(triggers)),
	trigger(triggers.size() > 0 ? triggers[0] : nullptr),
	useCustomBGColor(false)
{
	setSize(80, GlobalSettings::getInstance()->fontSize->floatValue() + 4);
	for (auto& t : triggers) t->addAsyncTriggerListener(this);
}

TriggerUI::~TriggerUI()
{
	for (auto& t : triggers)
	{
		if (!t.wasObjectDeleted()) t->removeAsyncTriggerListener(this);
	}
}

void TriggerUI::newMessage(const WeakReference<Trigger>& t)
{
	triggerTriggered(t.get());
}
