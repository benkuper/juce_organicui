#include "TargetParameter.h"
/*
  ==============================================================================

    TargetParameter.cpp
    Created: 2 Nov 2016 5:00:04pm
    Author:  bkupe

  ==============================================================================
*/


TargetParameter::TargetParameter(const String & niceName, const String & description, const String & initialValue, WeakReference<ControllableContainer> rootReference, bool enabled) :
	StringParameter(niceName, description, initialValue, enabled),
	targetType(CONTROLLABLE),
	useGhosting(true),
	showParameters(true),
	showTriggers(true),
	showFullAddressInEditor(false),
	showParentNameInEditor(true),
	maxDefaultSearchLevel(-1),
	defaultParentLabelLevel(3),
	rootContainer(nullptr),
	target(nullptr),
	targetContainer(nullptr),
	customGetTargetFunc(nullptr),
	customGetControllableLabelFunc(nullptr),
	customCheckAssignOnNextChangeFunc(nullptr),
	defaultContainerTypeCheckFunc(nullptr),
	customGetContainerLabelFunc(nullptr),
    customGetTargetContainerFunc(nullptr)

{
	showWarningInUI = true;

	lockManualControlMode = true;
	type = TARGET;

	setRootContainer(rootReference != nullptr?rootReference:Engine::mainEngine);
	
	scriptObject.setMethod("getTarget", TargetParameter::getTargetFromScript);

	argumentsDescription = "target";
}

TargetParameter::~TargetParameter()
{
	setRootContainer(nullptr);

	setTarget((ControllableContainer *)nullptr);
	setTarget((Controllable *)nullptr);
	setValue("");
}

void TargetParameter::setGhostValue(const String & ghostVal)
{
	if (ghostVal == ghostValue) return;
	ghostValue = ghostVal;
	if (ghostValue.isNotEmpty() && target == nullptr && targetContainer == nullptr) setWarningMessage("Link is broken !");
	
}

void TargetParameter::setValueFromTarget(Controllable * c, bool addToUndo)
{
	String newValue;

	if (c != nullptr)
	{

		if (target != nullptr && c == target.get())
		{
			String ca = target->getControlAddress();
			if (stringValue() == ca)
			{
				return;
			}
		}

		if (c->type < 0)
		{
			DBG("Target is not good !");
			return;
		}

		newValue = c->getControlAddress(rootContainer);
	}
	
	if(addToUndo) setUndoableValue(stringValue(), newValue);
	else setValue(newValue, false, true);
}

void TargetParameter::setValueFromTarget(ControllableContainer * cc, bool addToUndo)
{
	if (targetContainer != nullptr && cc == targetContainer.get())
	{
		String ca = targetContainer->getControlAddress();
		if (stringValue() == ca) return;
	}

	if (addToUndo) setUndoableValue(stringValue(), cc->getControlAddress(rootContainer));
	else setValue(cc->getControlAddress(rootContainer), false, true);
}

void TargetParameter::setValueInternal(var & newVal)
{
	StringParameter::setValueInternal(newVal);

	if (newVal.toString().isNotEmpty())
	{
		if (targetType == CONTAINER)
		{
			WeakReference<ControllableContainer> cc = rootContainer->getControllableContainerForAddress(newVal.toString(),true);
			if (cc != nullptr) setTarget(cc);
			else
			{
				setTarget((ControllableContainer *)nullptr);
				setGhostValue(newVal.toString());
			}
		} else
		{
			WeakReference<Controllable> c = rootContainer->getControllableForAddress(newVal.toString(),true);
			if (c != nullptr && !c.wasObjectDeleted()) setTarget(c);
			else
			{
				setTarget((Controllable *)nullptr);
				setGhostValue(newVal.toString());
			}
		}
	} else
	{
		if(targetType == CONTAINER) setTarget((ControllableContainer *)nullptr);
		else setTarget((ControllableContainer *)nullptr);

		setGhostValue("");
	}
}

void TargetParameter::setTarget(WeakReference<Controllable> c)
{
	if (target != nullptr)
	{
		if (!target.wasObjectDeleted())
		{
			target->removeInspectableListener(this);
			target->removeControllableListener(this);
		}
	}
	
	target = c;

	if (target != nullptr)
	{
		target->addInspectableListener(this);
		target->addControllableListener(this);
		setGhostValue("");
		clearWarning();
	}
	else
	{
		if (ghostValue.isNotEmpty()) setWarningMessage("Link is broken");
		else clearWarning();
	}
}

void TargetParameter::setTarget(WeakReference<ControllableContainer> cc)
{
	if (targetContainer != nullptr)
	{
		if (!targetContainer.wasObjectDeleted())
		{
			targetContainer->removeInspectableListener(this);
			targetContainer->removeControllableContainerListener(this);
		}
	}

	targetContainer = cc;

	if (targetContainer != nullptr)
	{
		targetContainer->addControllableContainerListener(this);
		targetContainer->addInspectableListener(this);

		setGhostValue("");
		clearWarning();
	}
	else
	{
		if (ghostValue.isNotEmpty()) setWarningMessage("Link is broken");
		else clearWarning();
	}
}

void TargetParameter::setRootContainer(WeakReference<ControllableContainer> newRootContainer)
{
	if (rootContainer == newRootContainer) return;

	if (rootContainer != nullptr && !rootContainer.wasObjectDeleted()) rootContainer->removeControllableContainerListener(this);

	rootContainer = newRootContainer;

	if(rootContainer != nullptr && !rootContainer.wasObjectDeleted()) rootContainer->addControllableContainerListener(this);

	setValue("");
}

void TargetParameter::childStructureChanged(ControllableContainer * cc)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;

	if (targetType == CONTROLLABLE)
	{
		if (target == nullptr)
		{
			if (ghostValue.isNotEmpty())
			{
				WeakReference<Controllable> c = rootContainer->getControllableForAddress(ghostValue);
				if (c != nullptr) setValueFromTarget(c);
			}
		} else
		{
			setValueFromTarget(target);
		}
	} else if (targetType == CONTAINER)
	{
		if (targetContainer == nullptr)
		{
			if (ghostValue.isNotEmpty())
			{
				WeakReference<ControllableContainer> tcc = rootContainer->getControllableContainerForAddress(ghostValue);
				if (tcc != nullptr) setValueFromTarget(tcc);
			}
		} else
		{
			setValueFromTarget(targetContainer);
		}
	}
}

void TargetParameter::inspectableDestroyed(Inspectable * i)
{
	String oldValue = stringValue();
	setValue("");
	setGhostValue(oldValue);
}


var TargetParameter::getJSONDataInternal()
{
	var data = StringParameter::getJSONDataInternal();
	if (value.toString().isEmpty() && ghostValue.isNotEmpty()) data.getDynamicObject()->setProperty("ghostValue", ghostValue);
	return data;
}

void TargetParameter::loadJSONDataInternal(var data)
{
	StringParameter::loadJSONDataInternal(data);
	if(data.hasProperty("ghostValue")) setGhostValue(data.getProperty("ghostValue", ""));

}

TargetParameterUI * TargetParameter::createTargetUI(TargetParameter * _target)
{
	return new TargetParameterUI(_target == nullptr ?this:_target);
}

ControllableUI * TargetParameter::createDefaultUI(Controllable * targetControllable)
{
	return createTargetUI((TargetParameter*)targetControllable);
}

var TargetParameter::getTargetFromScript(const juce::var::NativeFunctionArgs& a)
{
	TargetParameter * p = getObjectFromJS<TargetParameter>(a);
	if (p == nullptr) return var();
	if (p->targetType == CONTROLLABLE)
	{
		Controllable* c = p->target;
		if (c != nullptr) return c->getScriptObject();
	}
	else if (p->targetType == CONTAINER)
	{
		ControllableContainer* cc = p->targetContainer;
		if (cc != nullptr) return cc->getScriptObject();
	}

	return var();
}
