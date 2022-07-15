#pragma once

class DashboardFeedbackBroadcaster
{
public:
	virtual var getItemParameterFeedback(Parameter* p);

	void notifyParameterFeedback(WeakReference<Parameter> p);
	void notifyDataFeedback(var data);
	void notifyDashboardFeedback(var data);

	class FeedbackListener
	{
	public:
		/** Destructor. */
		virtual ~FeedbackListener() {}
		virtual void parameterFeedback(var data) = 0;
		virtual void dashboardFeedback(var data) = 0;
	};

	ListenerList<FeedbackListener> feedbackListeners;
	void addDashboardFeedbackListener(FeedbackListener* newListener) { feedbackListeners.add(newListener); }
	void removeDashboardFeedbackListener(FeedbackListener* listener) { feedbackListeners.remove(listener); }
};