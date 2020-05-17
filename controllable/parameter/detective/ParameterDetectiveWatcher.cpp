#include "ParameterDetectiveWatcher.h"

ParameterDetectiveWatcher::ParameterDetectiveWatcher(Parameter* p) :
	ControllableDetectiveWatcher(p),
	parameter(p)
{
	parameter->addParameterListener(this);
}

ParameterDetectiveWatcher::~ParameterDetectiveWatcher()
{
	if (!parameter.wasObjectDeleted()) parameter->removeParameterListener(this);
}

InspectableEditor* ParameterDetectiveWatcher::getEditor(bool isRoot)
{
	return new ParameterDetectiveWatcherEditor(this, isRoot);
}

void ParameterDetectiveWatcher::onExternalParameterValueChanged(Parameter* p)
{
	if (p == parameter) addValue(p->floatValue());
	ControllableDetectiveWatcher::onExternalParameterValueChanged(p);
}
