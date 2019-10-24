/*
  ==============================================================================

    TriggerUI.cpp
    Created: 8 Mar 2016 3:48:52pm
    Author:  bkupe

  ==============================================================================
*/

TriggerUI::TriggerUI(Trigger * trigger) :
    ControllableUI(trigger),
    trigger(trigger),
	useCustomBGColor(false)
{
	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);
    trigger->addAsyncTriggerListener(this);
}

TriggerUI::~TriggerUI()
{
    if(controllable.get())trigger->removeAsyncTriggerListener(this);
}

void TriggerUI::newMessage(const WeakReference<Trigger> & t)
{
	triggerTriggered(t.get());
}
