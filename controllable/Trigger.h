/*
 ==============================================================================

 Trigger.h
 Created: 8 Mar 2016 1:09:29pm
 Author:  bkupe

 ==============================================================================
 */
#pragma once


class TriggerButtonUI;
class TriggerImageUI;
class TriggerBlinkUI;

class Trigger : public Controllable
{
public:
	Trigger(const juce::String& niceName, const juce::String& description, bool enabled = true);
	~Trigger() { masterReference.clear(); }


	TriggerButtonUI* createButtonUI(juce::Array<Trigger*> triggers = {});
	TriggerImageUI* createImageUI(const juce::Image& image, bool keepSaturation = false, juce::Array<Trigger*> triggers = {});
	TriggerBlinkUI* createBlinkUI(juce::Array<Trigger*> triggers = {});
	ControllableUI* createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	virtual void trigger();

	virtual DashboardItem* createDashboardItem() override;

	// avoid feedback loop in listeners
	bool isTriggering;
public:
	//Listener
	class  TriggerListener
	{
	public:
		/** Destructor. */
		virtual ~TriggerListener() {}
		virtual void triggerTriggered(Trigger* p) = 0;
	};

	DECLARE_INSPECTACLE_SAFE_LISTENER(Trigger, trigger);
	DECLARE_ASYNC_EVENT(Trigger, Trigger, trigger, { TRIGGER_TRIGGERED }, EVENT_INSPECTABLE_CHECK);

	static Trigger* create() { return new Trigger("New Trigger", ""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Trigger"; }


private:
	juce::WeakReference<Trigger>::Master masterReference;
	friend class juce::WeakReference<Trigger>;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Trigger)
};