/*
 ==============================================================================

 Trigger.cpp
 Created: 8 Mar 2016 1:09:29pm
 Author:  bkupe

 ==============================================================================
 */

#include "JuceHeader.h"
#include "Trigger.h"

Trigger::Trigger(const String& niceName, const String& description, bool enabled) :
	Controllable(TRIGGER, niceName, description, enabled),
	isTriggering(false),
	triggerNotifier(5)
{
	scriptObject.getDynamicObject()->setMethod("trigger", Controllable::setValueFromScript);
}

TriggerButtonUI* Trigger::createButtonUI(Array<Trigger*> triggers)
{
	if (triggers.isEmpty()) triggers = { this };
	return new TriggerButtonUI(triggers);
}

TriggerImageUI* Trigger::createImageUI(const Image& image, bool keepSaturation, Array<Trigger*> triggers)
{
	if (triggers.isEmpty()) triggers = { this };
	return new TriggerImageUI(triggers, image, keepSaturation);
}

TriggerBlinkUI* Trigger::createBlinkUI(Array<Trigger*> triggers)
{
	if (triggers.isEmpty()) triggers = { this };
	return new TriggerBlinkUI(triggers);
}

ControllableUI* Trigger::createDefaultUI(Array<Controllable*> controllables)
{
	if (isControllableFeedbackOnly) return createBlinkUI(getArrayAs<Controllable, Trigger>(controllables));
	else return createButtonUI(getArrayAs<Controllable, Trigger>(controllables));
}

void Trigger::trigger(bool triggerSimilarSelected)
{
	if (!enabled || isTriggering) return;

	Array<WeakReference<Controllable>> relatedTriggers;
	if (triggerSimilarSelected)
		for (auto* related : getRelatedSelectedControllables())
			if (related != this) relatedTriggers.add(related);

	WeakReference<Controllable> safeThis(this);
	isTriggering = true;
	struct BailOutChecker
	{
		WeakReference<Controllable> trigger;
		bool shouldBailOut() const { return trigger == nullptr; }
	};
	triggerListeners.callChecked(BailOutChecker{ safeThis },
		&TriggerListener::triggerTriggered, this);
	if (safeThis == nullptr) return;
	triggerNotifier.addMessage(new TriggerEvent(TriggerEvent::TRIGGER_TRIGGERED, this));
	if (safeThis == nullptr) return;
	isTriggering = false;

	for (const auto& related : relatedTriggers)
		if (auto* trigger = dynamic_cast<Trigger*>(related.get())) trigger->trigger(false);
}

DashboardItem* Trigger::createDashboardItem()
{
	return new DashboardTriggerItem(this);
}
