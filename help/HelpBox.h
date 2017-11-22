#pragma once


class HelpBox :
	public Thread
{
public:
	juce_DeclareSingleton(HelpBox, true);

	HelpBox();
	~HelpBox();

	var helpData;
	
	String persistentDataID;
	String overDataID; 

	URL helpURL;

	void loadHelp(URL helpURL);
	void setOverData(String dataID);
	void setPersistentData(String dataID);

	const String getContentForID(String dataID);
	const String getCurrentData();

	class HelpListener
	{
	public:
		virtual ~HelpListener() {}
		virtual void helpContentChanged() {}
	};

	ListenerList<HelpListener> helpListeners;
	void addHelpListener(HelpListener* newListener) { helpListeners.add(newListener); }
	void removeHelpListener(HelpListener* listener) { helpListeners.remove(listener); }

	// Inherited via Thread
	virtual void run() override;
};
