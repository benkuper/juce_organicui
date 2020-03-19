/*
  ==============================================================================

    FloatParameterLabelUI.h
    Created: 10 Dec 2016 10:51:19am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class FloatParameterLabelUI :
	public ParameterUI, 
	public Label::Listener,
    public Timer
{
public:
	FloatParameterLabelUI(Parameter * p);
	virtual ~FloatParameterLabelUI() {};

	Label nameLabel;
	Label valueLabel;

	String prefix;
	String suffix;
    String valueString;

	float maxFontHeight;
    
    bool shouldUpdateLabel;

	const float pixelsPerUnit = 10; //1 = 10pixel
	float valueAtMouseDown;
	float valueOffsetSinceMouseDown;
	int lastMouseX;

	bool autoSize;
	void setAutoSize(bool value);

	void setPrefix(const String &_prefix);
	void setSuffix(const String &_suffix);

	virtual void feedbackStateChanged() override;

	virtual void updateTooltip() override;

	//void paint(Graphics &g) override;
	void resized() override;

	void mouseDownInternal(const MouseEvent &e) override;
	void mouseDrag(const MouseEvent &e) override;
	void mouseUpInternal(const MouseEvent &e) override;

	void updateUIParams() override;

protected:
	virtual void valueChanged(const var & v) override;
	virtual void labelTextChanged(Label * labelThatHasChanged) override;

    void timerCallback() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatParameterLabelUI)

};

class TimeLabel :
	public FloatParameterLabelUI
{
public:
	TimeLabel(Parameter * p);
	~TimeLabel();

protected:
	void valueChanged(const var &) override;
	void labelTextChanged(Label * l) override;


	String valueToTimeString(float val) const;
	float timeStringToValue(String str) const;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeLabel)
};