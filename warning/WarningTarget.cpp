#include "JuceHeader.h"

String WarningTarget::warningNoId = "";
String WarningTarget::warningAllId = "*";

WarningTarget::WarningTarget() :
	showWarningInUI(false),
	warningResolveInspectable(nullptr),
	warningTargetNotifier(10)
{
}

WarningTarget::~WarningTarget()
{
	if (warningMessage.size() > 0)
	{
		MessageManagerLock mmLock;
		WarningReporter::getInstance()->unregisterWarning(this);
	}

	if (warningTargetNotifier.isUpdatePending())
	{
		MessageManagerLock mmLock;
		warningTargetNotifier.handleUpdateNowIfNeeded();
		warningTargetNotifier.cancelPendingUpdate();
	}

	masterReference.clear();

}

void WarningTarget::setWarningMessage(const String& message, const String& id, bool log)
{
	if (WarningReporter::getInstanceWithoutCreating() == nullptr) return;
	if (message.isNotEmpty() && warningMessage.contains(id) && warningMessage[id] == message) return;

	if (message.isEmpty())
	{
		warningMessage.remove(id);
		if (warningMessage.size() == 0) WarningReporter::getInstance()->unregisterWarning(this);
	}
	else
	{
		if (log) LOGWARNING(message);
		warningMessage.set(id, message);
		WarningReporter::getInstance()->registerWarning(this);
	}

	notifyWarningChanged();
}

void WarningTarget::clearWarning(const String& id)
{
	if (id == warningAllId)
	{
		HashMap<String, String>::Iterator it(warningMessage);
		while (it.next()) clearWarning(it.getKey());
		return;
	}

	setWarningMessage(String(), id);
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

String WarningTarget::getWarningMessage(const String& id) const
{
	if (warningMessage.size() == 0) return "";

	String result;
	if (id == warningAllId)
	{
		HashMap<String, String>::Iterator it(warningMessage);
		while (it.next()) result += (it.getKey() != warningNoId ? "[" + it.getKey() + "] " : "") + it.getValue();
	}
	else if (warningMessage.contains(id))
	{
		result = warningMessage[id];
	}

	return result;
}

String WarningTarget::getWarningTargetName() const
{
	return "Unknown";
}
