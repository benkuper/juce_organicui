
WarningTarget::WarningTarget() :
	warningResolveInspectable(nullptr),
	warningTargetNotifier(10),
	showWarningInUI(false)
{
}

WarningTarget::~WarningTarget()
{
	clearWarning();
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
}

String WarningTarget::getWarningMessage() const
{
	return warningMessage;
}
