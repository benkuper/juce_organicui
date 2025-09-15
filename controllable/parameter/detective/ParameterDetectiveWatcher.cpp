#include "ParameterDetectiveWatcher.h"

ParameterDetectiveWatcher::ParameterDetectiveWatcher() :
	ControllableDetectiveWatcher(),
	parameter(nullptr)
{
	listUISize->setValue(100);
}

ParameterDetectiveWatcher::~ParameterDetectiveWatcher()
{
}


void ParameterDetectiveWatcher::setControllable(Controllable* c)
{
	if (parameter != nullptr)
	{
		parameter->removeParameterListener(this);
	}

	ControllableDetectiveWatcher::setControllable(c);

	if (controllable != nullptr && !controllable.wasObjectDeleted()) parameter = (Parameter*)controllable.get();
	else parameter = nullptr;

	if (parameter != nullptr)
	{
		parameter->addParameterListener(this);
		oldestVal = parameter->value.clone();
	}
}

void ParameterDetectiveWatcher::onExternalParameterValueChanged(Parameter* p)
{
	if (p == parameter) addValue(p->value.clone());
	ControllableDetectiveWatcher::onExternalParameterValueChanged(p);
}

IMPLEMENT_UI_FUNC(ParameterDetectiveWatcher);