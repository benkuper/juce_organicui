#pragma once

class Detective :
	public Manager<ControllableDetectiveWatcher>
{
public:
	juce_DeclareSingleton(Detective, true);

	Detective();
	~Detective();

	void watchControllable(Controllable* c);
	ControllableDetectiveWatcher* getItemForControllable(Controllable* c);

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Detective)
};