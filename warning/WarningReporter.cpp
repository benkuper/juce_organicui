#include "JuceHeader.h"
#include "WarningReporter.h"

juce_ImplementSingleton(WarningReporter)

WarningReporter::WarningReporter() :
	warningReporterNotifier(50)
{
	Engine::mainEngine->addEngineListener(this);
}

WarningReporter::~WarningReporter()
{
	clear();
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}

void WarningReporter::clear()
{
	for(auto & target : targets)
	{
		if (Inspectable* i = dynamic_cast<Inspectable*>(target.get()))
		{
			i->removeInspectableListener(this);
		}
	}
	targets.clear();
}

void WarningReporter::registerWarning(WeakReference<WarningTarget> target)
{
	if (Engine::mainEngine->isClearing) return;
	GenericScopedLock lock(targets.getLock());

	if (target == nullptr || target.wasObjectDeleted() || targets.contains(target)) return;
	targets.addIfNotAlreadyThere(target);
	
	if(Controllable* c = dynamic_cast<Controllable*>(target.get())) targetAddressMap.set(target, c->getControlAddress());
	else if (ControllableContainer* cc = dynamic_cast<ControllableContainer*>(target.get())) targetAddressMap.set(target, cc->getControlAddress());

	if (Inspectable* i = dynamic_cast<Inspectable*>(target.get())) i->addInspectableListener(this);
	warningReporterNotifier.addMessage(new WarningReporterEvent(WarningReporterEvent::WARNING_REGISTERED, target, targetAddressMap[target]));
}

void WarningReporter::unregisterWarning(WeakReference<WarningTarget> target)
{
	if (Engine::mainEngine->isClearing && target != Engine::mainEngine) return;
	GenericScopedLock lock(targets.getLock());

	if (target == nullptr || target.wasObjectDeleted() || !targets.contains(target)) return;
	targets.removeAllInstancesOf(target);
	String address = targetAddressMap.contains(target) ? targetAddressMap[target] : String();
	warningReporterNotifier.addMessage(new WarningReporterEvent(WarningReporterEvent::WARNING_UNREGISTERED, target, targetAddressMap[target]));

	targetAddressMap.remove(target);

}

void WarningReporter::fileLoaded()
{
	if (targets.size() > 0)
	{
		LOGWARNING("You have " << targets.size() << " warnings after loading the file,\nplease check the Warnings windows to resolve them.");
	}
}

void WarningReporter::inspectableDestroyed(Inspectable* i)
{
	unregisterWarning(i);
}
