#include "Trigger.h"
/*
 ==============================================================================

 Trigger.cpp
 Created: 8 Mar 2016 1:09:29pm
 Author:  bkupe

 ==============================================================================
 */


//#include "JsHelpers.h"

Trigger::Trigger(const String & niceName, const String &description, bool enabled) :
Controllable(TRIGGER, niceName, description, enabled),
isTriggering(false),
queuedNotifier(5)
{
	scriptObject.setMethod("trigger", Controllable::setValueFromScript);
}

TriggerButtonUI * Trigger::createButtonUI()
{
    return new TriggerButtonUI(this);
}

TriggerImageUI * Trigger::createImageUI(const Image &image)
{
	return new TriggerImageUI(this, image);
}

TriggerBlinkUI * Trigger::createBlinkUI()
{
	return new TriggerBlinkUI(this);
}

ControllableUI * Trigger::createDefaultUI(){
	if (isControllableFeedbackOnly) return createBlinkUI();
	else return createButtonUI();
}

void Trigger::trigger()
{
	if (enabled && !isTriggering) {
		isTriggering = true;
		listeners.call(&Listener::triggerTriggered, this);
		queuedNotifier.addMessage(new WeakReference<Trigger>(this));
		isTriggering = false;
	}
}

DashboardItem* Trigger::createDashboardItem()
{
	return new DashboardTriggerItem(this);
}
