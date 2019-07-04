
juce_ImplementSingleton(WarningReporter)

WarningReporter::WarningReporter() :
	warningReporterNotifier(50)
{
}

WarningReporter::~WarningReporter()
{
}

void WarningReporter::registerWarning(WarningTarget* target)
{
	if (target == nullptr || targets.contains(target)) return;
	targets.addIfNotAlreadyThere(target);
	warningReporterNotifier.addMessage(new WarningReporterEvent(WarningReporterEvent::WARNING_REGISTERED, target));
}

void WarningReporter::unregisterWarning(WarningTarget* target)
{
	if (target == nullptr || !targets.contains(target)) return;
	targets.removeAllInstancesOf(target);
	warningReporterNotifier.addMessage(new WarningReporterEvent(WarningReporterEvent::WARNING_UNREGISTERED, target));

}
