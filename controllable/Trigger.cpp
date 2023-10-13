/*
 ==============================================================================

 Trigger.cpp
 Created: 8 Mar 2016 1:09:29pm
 Author:  bkupe

 ==============================================================================
 */

#include "JuceHeader.h"

Trigger::Trigger(const String& niceName, const String& description, bool enabled) :
	Controllable(TRIGGER, niceName, description, enabled),
	isTriggering(false),
	queuedNotifier(5)
{
	scriptObject.getDynamicObject()->setMethod("trigger", Controllable::setValueFromScript);
}

TriggerButtonUI* Trigger::createButtonUI(Array<Trigger *> triggers)
{
	if (triggers.isEmpty()) triggers = { this };
	return new TriggerButtonUI(triggers);
}

TriggerImageUI* Trigger::createImageUI(const Image& image, bool keepSaturation, Array<Trigger *> triggers)
{
	if (triggers.isEmpty()) triggers = { this };
	return new TriggerImageUI(triggers, image, keepSaturation);
}

TriggerBlinkUI* Trigger::createBlinkUI(Array<Trigger *> triggers)
{
	if (triggers.isEmpty()) triggers = { this };
	return new TriggerBlinkUI(triggers);
}

ControllableUI* Trigger::createDefaultUI(Array<Controllable *> controllables)
{
	if (isControllableFeedbackOnly) return createBlinkUI(getArrayAs<Controllable, Trigger>(controllables));
	else return createButtonUI(getArrayAs<Controllable, Trigger>(controllables));
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
