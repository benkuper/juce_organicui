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

	bool addToUndoOnMouseUp;
	
	bool changeParamOnMouseUpOnly;
    bool assignOnMousePosDirect;
    float scaleFactor;

    int fixedDecimals;

    //interaction
    float initValue;

    //drawing checks
    int updateRate;
    float shouldRepaint;
    int lastDrawPos;
    String lastValueText;

    virtual void paint(Graphics &g) override;
    virtual void mouseDownInternal(const MouseEvent &e) override;
    virtual void mouseDrag(const MouseEvent &e) override;
    virtual void mouseUpInternal(const MouseEvent &e) override;

    virtual float getValueFromMouse();
    virtual float getValueFromPosition(const Point<int> &pos);
    int getDrawPos();
    virtual String getValueText() const;

    virtual void setParamNormalizedValueUndoable(float oldValue, float newValue);
    virtual void setParamNormalizedValue(float value);
    virtual float getParamNormalizedValue();
    virtual void rangeChanged(Parameter *)override;

    virtual void visibilityChanged() override;
    virtual void timerCallback() override;

	virtual void focusGained(FocusChangeType cause) override;

protected:
    virtual void valueChanged(const var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatSliderUI)
};
