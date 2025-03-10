/*
  ==============================================================================

    GapGrabber.h
    Created: 2 May 2016 5:40:46pm
    Author:  bkupe

  ==============================================================================
*/


#pragma once

class GapGrabber :
	public UITimerTarget,
	public juce::Component
{
public:
	enum Direction { HORIZONTAL, VERTICAL };

	GapGrabber(Direction _direction);
	virtual ~GapGrabber();

	void paint(juce::Graphics & g) override;
	void mouseDrag(const juce::MouseEvent &e) override;
	void mouseUp(const juce::MouseEvent &e) override;
	void mouseDown(const juce::MouseEvent &e) override;
	void handlePaintTimerInternal() override;

	Direction direction;
	int position;

	//Listener
	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void grabberGrabUpdate(GapGrabber *, int relativeDist) = 0;
		virtual void grabberGrabEvent(GapGrabber *, bool isGrabbing) {}
	};

	juce::ListenerList<Listener> listeners;
	void addGrabberListener(Listener* newListener) { listeners.add(newListener); }
	void removeGrabberListener(Listener* listener) { listeners.remove(listener); }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GapGrabber)
};

