#include "WarningTarget.h"

WarningTarget::WarningTarget() :
	warningResolveInspectable(nullptr),
	showWarningInUI(false),
    warningTargetNotifier(10)

{
}

WarningTarget::~WarningTarget()
{
	clearWarning();
	
	if (warningTargetNotifier.isUpdatePending())
	{
		warningTargetNotifier.triggerAsyncUpdate();
		uint32 t = Time::getApproximateMillisecondCounter();
		while (warningTargetNotifier.isUpdatePending())
		{
			if (Time::getApproximateMillisecondCounter() - t > 1000)
			{
				jassertfalse;
				break;
			}
		}
		warningTargetNotifier.cancelPendingUpdate();
	}

	masterReference.clear();
}

void WarningTarget::setWarningMessage(StringRef message)
{
	if (WarningReporter::getInstanceWithoutCreating() == nullptr) return;
	if (warningMessage == message) return;

	warningMessage = message;
	if(warningMessage.isEmpty()) WarningReporter::getInstance()->unregisterWarning(this);
	else WarningReporter::getInstance()->registerWarning(this);
	notifyWarningChanged();
}

void WarningTarget::notifyWarningChanged()
{
	warningTargetNotifier.addMessage(new WarningTargetEvent(WarningTargetEvent::WARNING_CHANGED, this));
}

void WarningTarget::resolveWarning()
{
	if (warningResolveInspectable != nullptr) warningResolveInspectable->selectThis();
	else
	{
		Inspectable* i = dynamic_cast<Inspectable*>(this);
		if (i != nullptr) i->selectThis();
	}
}

String WarningTarget::getWarningMessage() const
{
	return warningMessage;
}

String WarningTarget::getWarningTargetName() const 
{ 
	return "Unknown";
}
