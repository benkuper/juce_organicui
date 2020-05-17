
ParameterDetectiveWatcher::ParameterDetectiveWatcher(Parameter* p) :
	ControllableDetectiveWatcher(p),
	parameter(p)
{
	oldestVal = parameter->value.clone();
	parameter->addParameterListener(this);
	listUISize->setValue(100);
}

ParameterDetectiveWatcher::~ParameterDetectiveWatcher()
{
	if (!parameter.wasObjectDeleted()) parameter->removeParameterListener(this);
}


void ParameterDetectiveWatcher::onExternalParameterValueChanged(Parameter* p)
{
	if (p == parameter) addValue(p->value.clone());
	ControllableDetectiveWatcher::onExternalParameterValueChanged(p);
}

ControllableDetectiveWatcherUI* ParameterDetectiveWatcher::getUI()
{
	return new ParameterDetectiveWatcherUI(this);
}
