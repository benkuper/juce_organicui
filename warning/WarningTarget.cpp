#include "JuceHeader.h"
#include "WarningTarget.h"

String WarningTarget::warningNoId = "";
String WarningTarget::warningAllId = "*";

WarningTarget::WarningTarget() :
	showWarningInUI(false),
	warningResolveInspectable(nullptr),
	warningTargetNotifier(10),
	warningMessage(1)
{

}

WarningTarget::~WarningTarget()
{
	unregisterWarningNow();
	masterReference.clear();

}

void WarningTarget::setWarningMessage(const String& message, const String& id, bool log)
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isClearing) return;
	if (WarningReporter::getInstanceWithoutCreating() == nullptr) return;

	if (warningMessage.contains(id))
	{
		if (warningMessage[id] == message) return;
		warningMessage.remove(id);
	}
	else
	{
		if (message.isEmpty()) return;
	}

	if (warningMessage.size() == 0) WarningReporter::getInstance()->unregisterWarning(this);

	if (log && Engine::mainEngine != nullptr && !Engine::mainEngine->isLoadingFile && !Engine::mainEngine->isClearing)
	{
		String n = "Warning Target";
		if (ControllableContainer* cc = dynamic_cast<ControllableContainer*>(this))  n = cc->niceName;
		else if (Controllable* c = dynamic_cast<Controllable*>(this)) n = c->niceName;

		String prefix = id.isNotEmpty() ? "[" + id + "] " : "";
		NLOGWARNING(n,prefix + message);
	}

	if (!message.isEmpty())
	{
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

	setWarningMessage(String(), id, false);
}

void WarningTarget::unregisterWarningNow()
{
	if (warningMessage.size() > 0)
	{
		if (WarningReporter::getInstanceWithoutCreating())
		{
			if (!WarningReporter::getInstance()->targets.contains(this)) return;

			MessageManagerLock mmLock;
			WarningReporter::getInstance()->unregisterWarning(this);
		}
	}

	if (warningTargetNotifier.isUpdatePending())
	{
		MessageManagerLock mmLock;
		warningTargetNotifier.handleUpdateNowIfNeeded();
		warningTargetNotifier.cancelPendingUpdate();
	}

	warningMessage.clear();
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
		while (it.next()) result += (result.isNotEmpty() ? "\n" : "") + (it.getKey() != warningNoId ? "[" + it.getKey() + "] " : "") + it.getValue();
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
