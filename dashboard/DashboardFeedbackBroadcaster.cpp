
var DashboardFeedbackBroadcaster::getItemParameterFeedback(Parameter* p)
{
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("feedbackType", "uiFeedback");
	data.getDynamicObject()->setProperty("controlAddress", p->getControlAddress(DashboardManager::getInstance()));
	data.getDynamicObject()->setProperty("type", p->getTypeString());

	if (p->type == Parameter::ENUM)
	{
		EnumParameter* ep = (EnumParameter*)p;
		data.getDynamicObject()->setProperty("value", p->enabled ? ep->getValueData() : var());
	}
	else
	{
		data.getDynamicObject()->setProperty("value", p->enabled ? p->value : var());
	}

	if (p->canBeDisabledByUser) data.getDynamicObject()->setProperty("enabled", p->enabled);

	return data;
}

void DashboardFeedbackBroadcaster::notifyParameterFeedback(WeakReference<Parameter> p)
{
	if (p == nullptr || p.wasObjectDeleted()) return;
	notifyDataFeedback(getItemParameterFeedback(p));
}

void DashboardFeedbackBroadcaster::notifyDataFeedback(var data)
{
	feedbackListeners.call(&FeedbackListener::parameterFeedback, data);
}
