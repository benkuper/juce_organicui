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

	void setWarningMessage(StringRef message = String());
	void clearWarning() { setWarningMessage(); }

	void notifyWarningChanged();

	virtual void resolveWarning();
	virtual String getWarningMessage() const;
	virtual String getWarningTargetName() const;


	WeakReference<WarningTarget>::Master masterReference;

private:
	String warningMessage;

public:
	// ASYNC
	class  WarningTargetEvent
	{
	public:
		enum Type { WARNING_CHANGED };

		WarningTargetEvent(Type t, WarningTarget* target) :
			type(t), target(target) {}

		Type type;
		WeakReference<WarningTarget> target;
	};

	QueuedNotifier<WarningTargetEvent> warningTargetNotifier;
	typedef QueuedNotifier<WarningTargetEvent>::Listener AsyncListener;

	void addAsyncWarningTargetListener(AsyncListener* newListener) { warningTargetNotifier.addListener(newListener); }
	void addAsyncCoalescedWarningTargetListener(AsyncListener* newListener) { warningTargetNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncWarningTargetListener(AsyncListener* listener) { warningTargetNotifier.removeListener(listener); }
};
