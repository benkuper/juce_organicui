/*
  ==============================================================================

    FloatSliderUI.h
    Created: 8 Mar 2016 3:46:27pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class FloatSliderUI :
	public ParameterUI,
	public Timer
{

public:
    FloatSliderUI(Parameter * parameter = nullptr);
    virtual ~FloatSliderUI();

    enum Direction { HORIZONTAL, VERTICAL };

    //settings

    Direction orientation;
	Colour bgColor;
	Colour customColor;
	bool useCustomColor;
	
	bool addToUndoOnMouseUp;
	
	bool changeParamOnMouseUpOnly;
    bool assignOnMousePosDirect;
    float scaleFactor;

    int fixedDecimals;

    //interaction
    float initValue;

	//drawing checks
	float shouldRepaint;

	void setFrontColor(Colour c);
	void resetFrontColor();
	
	void paint(Graphics &g) override;
    void mouseDownInternal(const MouseEvent &e) override;
    void mouseDrag(const MouseEvent &e) override;
    void mouseUpInternal(const MouseEvent &e) override;

    float getValueFromMouse();
    float getValueFromPosition(const Point<int> &pos);

	virtual void setParamNormalizedValueUndoable(float oldValue, float newValue);
    virtual void setParamNormalizedValue(float value);
    virtual float getParamNormalizedValue();
	void rangeChanged(Parameter *)override;

	void visibilityChanged() override;

	void timerCallback() override;

protected:
    void valueChanged(const var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatSliderUI)
};