#pragma once

class DashboardFeedbackBroadcaster
{
public:
	DashboardFeedbackBroadcaster() : isBroadcasterBeingDestroyed(false) {}
    virtual ~DashboardFeedbackBroadcaster() 
	{
		isBroadcasterBeingDestroyed = true;
	}

    virtual juce::var getItemParameterFeedback(Parameter* p);
    
	void notifyParameterFeedback(juce::WeakReference<Parameter> p);
	void notifyDataFeedback(juce::var data);
	void notifyDashboardFeedback(juce::var data);

	bool isBroadcasterBeingDestroyed = false;

	class FeedbackListener
	{
	public:
		/** Destructor. */
		virtual ~FeedbackListener() {}
		virtual void parameterFeedback(juce::var data) = 0;
		virtual void dashboardFeedback(juce::var data) = 0;
	};

	juce::ListenerList<FeedbackListener, juce::Array<FeedbackListener*, juce::CriticalSection>> feedbackListeners;
	void addDashboardFeedbackListener(FeedbackListener* newListener) { feedbackListeners.add(newListener); }
	void removeDashboardFeedbackListener(FeedbackListener* listener) { if(!isBroadcasterBeingDestroyed) feedbackListeners.remove(listener); }
};
