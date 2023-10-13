/*
  ==============================================================================

    DashboardUI.h
    Created: 19 Apr 2017 11:06:43pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardUI :
	public BaseItemUI<Dashboard>,
	public Dashboard::AsyncListener
{
public:
	DashboardUI(Dashboard * item);
	~DashboardUI();

	void paint(juce::Graphics& g) override;

	void newMessage(const Dashboard::DashboardEvent& e) override;
};