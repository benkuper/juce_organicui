juce_ImplementSingleton(WarningReporter)

WarningReporter::WarningReporter() :
	warningReporterNotifier(50)
{
	Engine::mainEngine->addEngineListener(this);
}

WarningReporter::~WarningReporter()
{
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}

void WarningReporter::clear()
{
	targets.clear();
}

void WarningReporter::registerWarning(WeakReference<WarningTarget> target)
{
	if (Engine::mainEngine->isClearing) return;
	if (target == nullptr || target.wasObjectDeleted() || targets.contains(target)) return;
	targets.addIfNotAlreadyThere(target);
	warningReporterNotifier.addMessage(new WarningReporterEvent(WarningReporterEvent::WARNING_REGISTERED, target));
}

void WarningReporter::unregisterWarning(WeakReference<WarningTarget> target)
{
	if (Engine::mainEngine->isClearing) return;
	if (target == nullptr || target.wasObjectDeleted() || !targets.contains(target)) return;
	targets.removeAllInstancesOf(target);
	warningReporterNotifier.addMessage(new WarningReporterEvent(WarningReporterEvent::WARNING_UNREGISTERED, target));

}

void WarningReporter::endLoadFile()
{
	if (targets.size() > 0)
	{
		LOGWARNING("You have " << targets.size() << " warnings after loading the file,\nplease check the Warnings windows to resolve them.");
	}
}
