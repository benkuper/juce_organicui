#pragma once

class DashboardFeedbackBroadcaster
{
public:
    virtual ~DashboardFeedbackBroadcaster(){}
    virtual juce::var getItemParameterFeedback(Parameter* p);
    
	void notifyParameterFeedback(juce::WeakReference<Parameter> p);
	void notifyDataFeedback(juce::var data);
	void notifyDashboardFeedback(juce::var data);

	class FeedbackListener
	{
	public:
		/** Destructor. */
		virtual ~FeedbackListener() {}
		virtual void parameterFeedback(juce::var data) = 0;
		virtual void dashboardFeedback(juce::var data) = 0;
	};

	juce::ListenerList<FeedbackListener> feedbackListeners;
	void addDashboardFeedbackListener(FeedbackListener* newListener) { feedbackListeners.add(newListener); }
	void removeDashboardFeedbackListener(FeedbackListener* listener) { feedbackListeners.remove(listener); }
};
