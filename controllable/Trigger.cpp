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
	if (enabled && !isTriggering) {
		isTriggering = true;
		triggerListeners.call(&TriggerListener::triggerTriggered, this);
		triggerNotifier.addMessage(new TriggerEvent(TriggerEvent::TRIGGER_TRIGGERED, this));
		isTriggering = false;

		if (triggerSimilarSelected && InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 1)
		{
			if (ControllableContainer* cc = getSelectedParentInHierarchy())
			{
				String relativeAddress = getControlAddress(cc);
				Array<ControllableContainer*> containers = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<ControllableContainer>();
				containers.removeAllInstancesOf(cc);

				for (ControllableContainer* c : containers)
				{
					if (Trigger* t = dynamic_cast<Trigger*>(c->getControllableForAddress(relativeAddress)))
					{
						t->trigger(false);
					}
				}
			}
		}
	}
}

DashboardItem* Trigger::createDashboardItem()
{
	return new DashboardTriggerItem(this);
}
