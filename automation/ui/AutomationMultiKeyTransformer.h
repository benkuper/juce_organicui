/*
  ==============================================================================

    AutomationMultiKeyTransformer.h
    Created: 25 Mar 2017 4:14:45pm
    Author:  Ben

  ==============================================================================
*/

#pragma once  
class AutomationUI;

class AutomationMultiKeyTransformer :
	public juce::Component
{
public:
	AutomationMultiKeyTransformer(AutomationUI * aui, juce::Array<AutomationKeyUI *> keysUI);
	~AutomationMultiKeyTransformer();

	AutomationUI * aui;
	juce::Array<AutomationKeyUI *> keysUI;
	juce::Array<juce::Point<float>> keysRelativePositions;
	juce::Array<juce::Point<float>> keysTimesAndValuesPositions;

	juce::ResizableBorderComponent resizer;
	
	juce::Rectangle<int> keyBounds;
	juce::Point<int> posAtMouseDown;

	void updateBoundsFromKeys();
	void updateKeysFromBounds();

	void parentHierarchyChanged() override;

	void resized() override;
	void paint(juce::Graphics &g) override;

	void mouseDown(const juce::MouseEvent &e) override;
	void mouseDrag(const juce::MouseEvent &e) override;
	void mouseUp(const juce::MouseEvent &e) override;
};

