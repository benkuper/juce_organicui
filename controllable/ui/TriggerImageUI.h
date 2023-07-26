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
	public juce::Timer
{
public:
	TriggerImageUI(juce::Array<Trigger *>, const juce::Image &image, bool keepSaturation = false);
	~TriggerImageUI();

	juce::Image onImage;
	juce::Image offImage;

	bool drawTriggering;
	bool forceDrawTriggering;

	void paint(juce::Graphics &g) override;
	void triggerTriggered(const Trigger * p) override;

	void mouseDownInternal(const juce::MouseEvent &e) override;


	virtual void timerCallback() override;
};

