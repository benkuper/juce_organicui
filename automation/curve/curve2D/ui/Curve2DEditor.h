/*
  ==============================================================================

    Curve2DEditor.h
    Created: 5 Feb 2017 2:58:27pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class Curve2DEditor :
	public GenericControllableContainerEditor
{
public:
	Curve2DEditor(Curve2D * curve, bool isRoot);
	~Curve2DEditor();

	Curve2D * curve;
	std::unique_ptr<Curve2DUI> curveUI;
	std::unique_ptr<FloatSliderUI> positionUI;
	std::unique_ptr<ParameterUI> valueUI;

	void resizedInternalHeader(juce::Rectangle<int>& r) override;
	void resizedInternalContent(juce::Rectangle<int> &r) override;


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Curve2DEditor)
};



#pragma once
