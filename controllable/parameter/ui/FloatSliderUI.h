/*
  ==============================================================================

    FloatSliderUI.h
    Created: 8 Mar 2016 3:46:27pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class FloatSliderUI :
	public ParameterUI
{

public:
    FloatSliderUI(juce::Array<Parameter *> parameters);
    virtual ~FloatSliderUI();

    enum Direction { HORIZONTAL, VERTICAL, ROTARY };

    //settings

    Direction orientation;

	bool addToUndoOnMouseUp;
	
	bool changeParamOnMouseUpOnly;
    bool assignOnMousePosDirect;

    int fixedDecimals;
    juce::String prefix;
    juce::String suffix;

    //interaction
    float initValue;
    float initNormalizedValue;

    int lastDrawPos;
    juce::String lastValueText;

    virtual void paint(juce::Graphics& g) override;
    virtual void drawBG(juce::Graphics &g);
    virtual void mouseDownInternal(const juce::MouseEvent& e) override;
    virtual void mouseDrag(const juce::MouseEvent& e) override;
    virtual void mouseUpInternal(const juce::MouseEvent &e) override;

    virtual float getNormalizedValueFromMouse();
    virtual float getNormalizedValueFromMouseDrag(const juce::MouseEvent &e);
    virtual float getNormalizedValueFromPosition(const juce::Point<int> &pos);
    virtual int getDrawPos();
    virtual juce::String getValueText() const;

    virtual float getAngleForValue(float val, float angleReduction);
    void drawRotary(juce::Graphics &g, juce::Colour c, float startPos, float endPos, float reduction = 0);

    virtual void setParamNormalizedValueUndoable(float oldValue, float newValue);
    virtual void setParamNormalizedValue(float value);
    virtual float getParamNormalizedValue();
    virtual void rangeChanged(Parameter *)override;

	virtual void focusGained(FocusChangeType cause) override;

protected:
    virtual void valueChanged(const juce::var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatSliderUI)
};
