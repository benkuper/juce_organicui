/*
  ==============================================================================

    GradientColorUI.h
    Created: 11 Apr 2017 11:40:31am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class GradientColorUI :
	public BaseItemMinimalUI<GradientColor>
{
public:
	GradientColorUI(GradientColor * item);
	~GradientColorUI();

	std::unique_ptr<ColorParameterUI> colorUI;
	Path drawPath;

	float posAtMouseDown; 

	void paint(Graphics &g) override;
	void resized() override;

	void mouseDown(const MouseEvent &e) override;
	void mouseUp(const MouseEvent &e) override;
};
