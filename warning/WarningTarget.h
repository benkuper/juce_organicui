/*
  ==============================================================================

	WarningTarget.h
	Created: 19 Apr 2017 10:57:53pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

class Inspectable;

class WarningTarget
{
public:
	WarningTarget();
	virtual ~WarningTarget();

	bool showWarningInUI;
	Inspectable* warningResolveInspectable;

	static juce::String warningNoId;
	static juce::String warningAllId;

	juce::HashMap<juce::String, juce::String> warningMessage;

	void setWarningMessage(const juce::String& message = juce::String(), const juce::String& id = warningNoId, bool log = true);
	void clearWarning(const juce::String& id = warningNoId);

	void unregisterWarningNow();

	void notifyWarningChanged();

	virtual void resolveWarning();
	virtual juce::String getWarningMessage(const juce::String& id = warningAllId) const;
	virtual juce::String getWarningTargetName() const;


	juce::WeakReference<WarningTarget>::Master masterReference;

	// ASYNC
	class  WarningTargetEvent
	{
	public:
		enum Type { WARNING_CHANGED };

		WarningTargetEvent(Type t, WarningTarget* target) :
			type(t), target(target) {}

		Type type;
		juce::WeakReference<WarningTarget> target;
	};

	QueuedNotifier<WarningTargetEvent> warningTargetNotifier;
	typedef QueuedNotifier<WarningTargetEvent>::Listener AsyncListener;

	void addAsyncWarningTargetListener(AsyncListener* newListener) { warningTargetNotifier.addListener(newListener); }
	void addAsyncCoalescedWarningTargetListener(AsyncListener* newListener) { warningTargetNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncWarningTargetListener(AsyncListener* listener) { warningTargetNotifier.removeListener(listener); }
};
