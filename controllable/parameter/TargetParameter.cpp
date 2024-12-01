/*
  ==============================================================================

	TargetParameter.cpp
	Created: 2 Nov 2016 5:00:04pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

using namespace juce;

TargetParameter::TargetParameter(const String& niceName, const String& description, const String& initialValue, WeakReference<ControllableContainer> rootReference, bool enabled) :
	StringParameter(niceName, description, initialValue, enabled),
	targetType(CONTROLLABLE),
	useGhosting(true),
	showFullAddressInEditor(false),
	showParentNameInEditor(true),
	maxDefaultSearchLevel(-1),
	defaultParentLabelLevel(3),
	isTryingFixingLink(false),
	manuallySettingNull(false),
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

	type = TARGET;

	setRootContainer(rootReference != nullptr ? rootReference : Engine::mainEngine);

	scriptObject.getDynamicObject()->setMethod("getTarget", TargetParameter::getTargetFromScript);

	defaultValue = "";
	argumentsDescription = "target";
}

TargetParameter::~TargetParameter()
{
	if (rootContainer != nullptr && !rootContainer.wasObjectDeleted()) rootContainer->removeControllableContainerListener(this);
	setRootContainer(nullptr, false, false);
	ghostValue = ""; //force not ghost to avoid launching a warning
	setValue("", true, false);
	setTarget((ControllableContainer*)nullptr);
	setTarget((Controllable*)nullptr);
}

void TargetParameter::resetValue(bool silentSet)
{
	manuallySettingNull = true;
	if (targetType == CONTAINER) setTarget((ControllableContainer*)nullptr);
	else setTarget((Controllable*)nullptr);
	setGhostValue("");
	setUndoableValue("");

	queuedNotifier.addMessage(new ParameterEvent(ParameterEvent::VALUE_CHANGED, this, getValue()));
	clearWarning();

	manuallySettingNull = false;
}

void TargetParameter::setGhostValue(const String& ghostVal)
{
	if (ghostVal == ghostValue) return;
	ghostValue = ghostVal;
	//if (ghostValue.isNotEmpty() && target == nullptr && targetContainer == nullptr)
	//{
	//	setWarningMessage("Link is broken !");
	//}

}

void TargetParameter::setValueFromTarget(Controllable* c, bool addToUndo, bool setSimilarSelected)
{
	String newValue;

	if (c != nullptr)
	{
		if (target != nullptr && c == target.get())
		{
			String ca = target->getControlAddress(rootContainer);
			if (stringValue() == ca)
			{
				return;
			}

			if (rootContainer != nullptr)
			{
				if (rootContainer->getControllableForAddress(ca) == nullptr)
				{
					//DBG("Back link broken !");
					return;
				}
			}
		}


		if (c->type < 0)
		{
			DBG("Target is not good !");
			return;
		}

		newValue = c->getControlAddress(rootContainer);
	}
	else
	{
		manuallySettingNull = true;
	}

	if (addToUndo) setUndoableValue(newValue, false, setSimilarSelected);
	else setValue(newValue, false, true, true, setSimilarSelected);

	manuallySettingNull = false;
}

void TargetParameter::setValueFromTarget(ControllableContainer* cc, bool addToUndo, bool setSimilarSelected)
{
	if (targetContainer != nullptr && cc == targetContainer.get())
	{
		String ca = targetContainer->getControlAddress(rootContainer);
		if (stringValue() == ca) return;

		if (rootContainer != nullptr)
		{
			if (rootContainer->getControllableContainerForAddress(ca) == nullptr)
			{
				//DBG("Back link broken !");
				return;
			}
		}
	}

	if (cc == nullptr)
	{
		manuallySettingNull = true;
	}

	if (addToUndo) setUndoableValue(cc->getControlAddress(rootContainer), false, setSimilarSelected);
	else setValue(cc->getControlAddress(rootContainer), false, true, true, setSimilarSelected);

	manuallySettingNull = false;
}

void TargetParameter::setValueInternal(var& newVal)
{
	StringParameter::setValueInternal(newVal);
	if (newVal.toString().isNotEmpty())
	{
		if (targetType == CONTAINER)
		{
			WeakReference<ControllableContainer> cc = rootContainer->getControllableContainerForAddress(newVal.toString(), true);
			if (cc != nullptr) setTarget(cc);
			else
			{
				setTarget((ControllableContainer*)nullptr);
				//setGhostValue(newVal.toString());
			}
		}
		else
		{
			WeakReference<Controllable> c = rootContainer->getControllableForAddress(newVal.toString(), true);
			if (c != nullptr && !c.wasObjectDeleted()) setTarget(c);
			else
			{
				setTarget((Controllable*)nullptr);
				//setGhostValue(newVal.toString());
			}
		}
	}
	else
	{
		if (targetType == CONTAINER) setTarget((ControllableContainer*)nullptr);
		else setTarget((ControllableContainer*)nullptr);

		//setGhostValue("");
	}

}

var TargetParameter::getCroppedValue(var val)
{
	return val.isString() ? val : "";
}

Controllable* TargetParameter::getTargetControllable() { return target.get(); }

Trigger* TargetParameter::getTargetTrigger() { return dynamic_cast<Trigger*>(target.get()); }

Parameter* TargetParameter::getTargetParameter() { return dynamic_cast<Parameter*>(target.get()); }

ControllableContainer* TargetParameter::getTargetContainer() { return targetContainer.get(); }

void TargetParameter::setTarget(WeakReference<Controllable> c)
{
	if (target != nullptr)
	{
		if (target == c) return;

		if (!target.wasObjectDeleted())
		{
			target->removeInspectableListener(this);
			target->removeControllableListener(this);
		}
	}

	previousTarget = target;

	target = c;

	if (target != nullptr)
	{
		target->addInspectableListener(this);
		target->addControllableListener(this);
		setGhostValue(target->getControlAddress(rootContainer.get()));

		if (getWarningMessage().isNotEmpty() && !isBeingDestroyed && !Engine::mainEngine->isLoadingFile && !Engine::mainEngine->isClearing)
		{
			LOG("Link is recovered " + ghostValue);
		}
		clearWarning();
		if (rootContainer != nullptr && !rootContainer.wasObjectDeleted()) rootContainer->removeControllableContainerListener(this);
	}
	else
	{
		if (value.toString().isNotEmpty()) setGhostValue(value.toString());
		if (ghostValue.isNotEmpty() && !isBeingDestroyed)
		{
			if (Engine::mainEngine->isLoadingFile && !isTryingFixingLink)
			{
				Engine::mainEngine->addEngineListener(this);
			}
			else
			{
				if (!Engine::mainEngine->isClearing && rootContainer != nullptr && !rootContainer.wasObjectDeleted())
				{
					if (!isBeingDestroyed && !manuallySettingNull) setWarningMessage("Link is broken : " + ghostValue);
					rootContainer->addControllableContainerListener(this);
				}
			}
		}
		else clearWarning();
	}
}

void TargetParameter::setTarget(WeakReference<ControllableContainer> cc)
{

	if (targetContainer != nullptr)
	{
		if (targetContainer == cc) return;

		if (!targetContainer.wasObjectDeleted())
		{
			targetContainer->removeInspectableListener(this);
			targetContainer->removeControllableContainerListener(this);
		}
	}

	previousTargetContainer = targetContainer;

	targetContainer = cc;

	if (targetContainer != nullptr)
	{
		targetContainer->addControllableContainerListener(this);
		targetContainer->addInspectableListener(this);

		setGhostValue(targetContainer->getControlAddress(rootContainer.get()));
		clearWarning();
		if (rootContainer != nullptr && !rootContainer.wasObjectDeleted()) rootContainer->removeControllableContainerListener(this);

	}
	else
	{
		if (value.toString().isNotEmpty()) setGhostValue(value.toString());
		if (ghostValue.isNotEmpty() && !isBeingDestroyed)
		{
			if (Engine::mainEngine->isLoadingFile && !isTryingFixingLink)
			{
				Engine::mainEngine->addEngineListener(this);
			}
			else
			{
				if (!isBeingDestroyed && !manuallySettingNull) setWarningMessage("Link is broken : " + ghostValue);
				if (!Engine::mainEngine->isClearing && rootContainer != nullptr && !rootContainer.wasObjectDeleted())
				{
					rootContainer->addControllableContainerListener(this);
				}
			}
		}
		else clearWarning();
	}
}

void TargetParameter::tryFixBrokenLink()
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;

	isTryingFixingLink = true;

	if (targetType == CONTROLLABLE)
	{
		if (target == nullptr)
		{
			if (ghostValue.isNotEmpty())
			{
				WeakReference<Controllable> c = rootContainer->getControllableForAddress(ghostValue);
				if (c != nullptr) setValueFromTarget(c);
				else
				{
					//NLOGWARNING(niceName, "Link is broken on load : " + ghostValue);
					setTarget((Controllable*)nullptr);
				}
			}
		}
		else
		{
			setValueFromTarget(target);
		}
	}
	else if (targetType == CONTAINER)
	{
		if (targetContainer == nullptr)
		{
			if (ghostValue.isNotEmpty())
			{
				WeakReference<ControllableContainer> tcc = rootContainer->getControllableContainerForAddress(ghostValue);
				if (tcc != nullptr) setValueFromTarget(tcc);
			}
		}
		else
		{
			setValueFromTarget(targetContainer);
		}
	}

	isTryingFixingLink = false;
}

void TargetParameter::setRootContainer(WeakReference<ControllableContainer> newRootContainer, bool engineIfNull, bool forceSetValue)
{
	if (rootContainer == newRootContainer) return;

	if (rootContainer != nullptr && !rootContainer.wasObjectDeleted())
	{
		rootContainer->removeControllableContainerListener(this);
	}

	if (newRootContainer == nullptr && engineIfNull) newRootContainer = Engine::mainEngine;
	rootContainer = newRootContainer;

	if (forceSetValue)
	{
		ghostValue = "";
		setValue(getValue(), false, true);
	}
}

void TargetParameter::childStructureChanged(ControllableContainer*)
{
	tryFixBrokenLink();
}

void TargetParameter::controllableControlAddressChanged(Controllable* c)
{
	if (target == c)
	{
		setValueFromTarget(target); //force update
	}
}

void TargetParameter::childAddressChanged(ControllableContainer* cc)
{
	if (targetContainer == cc)
	{
		setValueFromTarget(targetContainer);
	}
}

void TargetParameter::inspectableDestroyed(Inspectable* i)
{
	if ((targetType == CONTAINER && targetContainer == nullptr) || (targetType == CONTROLLABLE && target == nullptr))
	{
		//DBG("Inspectable destroyed, ghost value : " << ghostValue);
		//String oldValue = stringValue();
		setValue("");
		//setGhostValue(oldValue);
	}
}

bool TargetParameter::setAttributeInternal(String param, var attributeValue)
{
	if (param == "targetType") targetType = (attributeValue.toString().toLowerCase() == "container") ? CONTAINER : CONTROLLABLE;
	else if (param == "searchLevel") maxDefaultSearchLevel = jmax<int>(attributeValue, -1);
	else if (param == "allowedTypes")
	{
		typesFilter.clear();
		if (attributeValue.isString()) typesFilter.add(attributeValue.toString());
		else if (attributeValue.isArray())
		{
			for (int i = 0; i < attributeValue.size(); i++) typesFilter.add(attributeValue[i].toString());
		}
	}
	else if (param == "excludedTypes")
	{
		excludeTypesFilter.clear();
		if (attributeValue.isString()) excludeTypesFilter.add(attributeValue.toString());
		else if (attributeValue.isArray())
		{
			for (int i = 0; i < attributeValue.size(); i++) excludeTypesFilter.add(attributeValue[i].toString());
		}
	}
	else if (param == "root")
	{
		if (attributeValue.isObject())
		{
			ControllableContainer* cc = dynamic_cast<ControllableContainer*>((ControllableContainer*)(juce::int64)attributeValue.getDynamicObject()->getProperty(scriptPtrIdentifier));
			if (cc != nullptr) setRootContainer(cc);
		}
	}
	else if (param == "labelLevel")
	{
		defaultParentLabelLevel = jmax<int>(attributeValue, 1);
	}
	else
	{
		return Parameter::setAttributeInternal(param, attributeValue);
	}

	return true;
}

StringArray TargetParameter::getValidAttributes() const
{
	StringArray att = StringParameter::getValidAttributes();
	att.addArray({ "targetType", "searchLevel","allowedTypes","excludedTypes","root","labelLevel" });
	return att;
}


var TargetParameter::getJSONDataInternal()
{
	var data = StringParameter::getJSONDataInternal();
	if (value.toString().isEmpty() && ghostValue.isNotEmpty())
	{
		data.getDynamicObject()->setProperty("ghostValue", ghostValue);
	}
	return data;
}

void TargetParameter::loadJSONDataInternal(var data)
{
	ghostValue = data.getProperty("ghostValue", data.getProperty("value", ""));
	StringParameter::loadJSONDataInternal(data);
}

void TargetParameter::endLoadFile()
{
	Engine::mainEngine->removeEngineListener(this);
	if (target == nullptr && targetContainer == nullptr) tryFixBrokenLink();
}

TargetParameterUI* TargetParameter::createTargetUI(Array<TargetParameter*> parameters)
{
	if (parameters.size() == 0) parameters = { this };
	return new TargetParameterUI(parameters);
}

ControllableUI* TargetParameter::createDefaultUI(Array<Controllable*> controllables)
{
	return createTargetUI(getArrayAs<Controllable, TargetParameter>(controllables));
}

DashboardItem* TargetParameter::createDashboardItem()
{
	return new DashboardTargetParameterItem(this);
}

var TargetParameter::getTargetFromScript(const juce::var::NativeFunctionArgs& a)
{
	TargetParameter* p = getObjectFromJS<TargetParameter>(a);
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
