#pragma once

class WarningTargetUI :
	public Component,
	public SettableTooltipClient,
	public WarningTarget::AsyncListener
{
public:
	WarningTargetUI(WarningTarget* target);
	~WarningTargetUI();

	WarningTarget* target;
	void paint(Graphics& g) override;

	void newMessage(const WarningTarget::WarningTargetEvent& e) override;
};