/*
  ==============================================================================

    GradientColorUI.h
    Created: 11 Apr 2017 11:40:31am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class GradientColorUI :
	public ItemMinimalUI<GradientColor>
{
public:
	GradientColorUI(GradientColor * item);
	~GradientColorUI();

	std::unique_ptr<ColorParameterUI> colorUI;
	juce::Path drawPath;

	void paint(juce::Graphics &g) override;
	void resized() override;

	void mouseDown(const juce::MouseEvent &e) override;
};
