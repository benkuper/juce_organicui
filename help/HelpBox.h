#pragma once


class HelpBox :
	public juce::Thread
{
public:
	juce_DeclareSingleton(HelpBox, true);

	HelpBox();
	~HelpBox();

	juce::var helpData;
	
	juce::String persistentDataID;
	juce::String overDataID;

	juce::URL helpURL;

	void loadHelp();
	void loadLocalHelp();

	void setOverData(juce::String dataID);
	void clearOverData(juce::String dataID);

	void setPersistentData(juce::String dataID);
	void clearPersistentData(juce::String dataID);

	const juce::String getContentForID(juce::String dataID);
	const juce::String getCurrentData();

	class HelpListener
	{
	public:
		virtual ~HelpListener() {}
		virtual void helpContentChanged() {}
	};

	juce::ListenerList<HelpListener> helpListeners;
	void addHelpListener(HelpListener* newListener) { helpListeners.add(newListener); }
	void removeHelpListener(HelpListener* listener) { helpListeners.remove(listener); }

	// Inherited via Thread
	virtual void run() override;
};
