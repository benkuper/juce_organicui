#pragma once

class WarningTargetUI :
	public juce::Component,
	public juce::SettableTooltipClient,
	public WarningTarget::AsyncListener
{
public:
	WarningTargetUI(WarningTarget * target);
	~WarningTargetUI();

	juce::WeakReference<WarningTarget> target;
	void paint(juce::Graphics& g) override;


	void newMessage(const WarningTarget::WarningTargetEvent& e) override;
};