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
	WarningReporter* reporter = WarningReporter::getInstanceWithoutCreating();
	if (reporter == nullptr) return;

	{
		const ScopedLock lock(warningMessageLock);

		if (warningMessage.contains(id))
		{
			if (warningMessage[id] == message) return;
			warningMessage.remove(id);
		}
		else
		{
			if (message.isEmpty()) return;
		}

		if (warningMessage.size() == 0) reporter->unregisterWarning(this, id);

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
			reporter->registerWarning(this, id, message);
		}
	}

	notifyWarningChanged();
}

void WarningTarget::clearWarning(const String& id)
{
	if (id == warningAllId)
	{
		const StringArray warningIds = getWarningMessages().getAllKeys();
		for (const auto& warningId : warningIds)
			setWarningMessage(String(), warningId, false);
		return;
	}

	setWarningMessage(String(), id, false);
}

void WarningTarget::unregisterWarningNow()
{
	bool hasWarnings = false;
	{
		const ScopedLock lock(warningMessageLock);
		hasWarnings = warningMessage.size() > 0;
	}

	if (hasWarnings)
	{
		if (WarningReporter::getInstanceWithoutCreating())
		{
			if (WarningReporter::getInstance()->targets.contains(this))
			{
				MessageManagerLock mmLock;
				WarningReporter::getInstance()->unregisterWarning(this, warningAllId);
			}
		}
	}

	if (warningTargetNotifier.isUpdatePending())
	{
		MessageManagerLock mmLock;
		warningTargetNotifier.handleUpdateNowIfNeeded();
		warningTargetNotifier.cancelPendingUpdate();
	}

	const ScopedLock lock(warningMessageLock);
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
	const ScopedLock lock(warningMessageLock);
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

StringPairArray WarningTarget::getWarningMessages() const
{
	StringPairArray result(false);
	const ScopedLock lock(warningMessageLock);
	HashMap<String, String>::Iterator it(warningMessage);
	while (it.next()) result.set(it.getKey(), it.getValue());
	return result;
}

String WarningTarget::getWarningTargetName() const
{
	return "Unknown";
}
