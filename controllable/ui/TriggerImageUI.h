/*
  ==============================================================================

    TriggerImageUI.h
    Created: 4 Jan 2017 1:32:47pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class TriggerImageUI :
	public TriggerUI,
	public Timer
{
public:
	TriggerImageUI(Trigger *, const Image &image, bool keepSaturation = false);
	~TriggerImageUI();

	Image onImage;
	Image offImage;

	bool drawTriggering;
	bool forceDrawTriggering;

	void paint(Graphics &g) override;
	void triggerTriggered(const Trigger * p) override;

	void mouseDownInternal(const MouseEvent &e) override;


	virtual void timerCallback() override;
};

