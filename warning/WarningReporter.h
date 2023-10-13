/*
  ==============================================================================

    WarningReporter.h
    Created: 19 Apr 2017 10:57:53pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class WarningReporter :
	public EngineListener
{
public:
	juce_DeclareSingleton(WarningReporter, true);

	juce::Array<juce::WeakReference<WarningTarget>> targets;

	WarningReporter();
	~WarningReporter();

	void clear();

	void registerWarning(juce::WeakReference<WarningTarget>);
	void unregisterWarning(juce::WeakReference<WarningTarget>);

	void endLoadFile() override;

	// ASYNC
	class  WarningReporterEvent
	{
	public:
		enum Type { WARNING_REGISTERED, WARNING_UNREGISTERED };

		WarningReporterEvent(Type t, juce::WeakReference<WarningTarget> target) :
			type(t), target(target) {}

		Type type;
		juce::WeakReference<WarningTarget> target;
	};

	QueuedNotifier<WarningReporterEvent> warningReporterNotifier;
	typedef QueuedNotifier<WarningReporterEvent>::Listener AsyncListener;

	void addAsyncWarningReporterListener(AsyncListener* newListener) { warningReporterNotifier.addListener(newListener); }
	void addAsyncCoalescedWarningReporterListener(AsyncListener* newListener) { warningReporterNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncWarningReporterListener(AsyncListener* listener) { warningReporterNotifier.removeListener(listener); }

};
