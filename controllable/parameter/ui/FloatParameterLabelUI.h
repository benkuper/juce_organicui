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
	public juce::Label::Listener
{
public:
	FloatParameterLabelUI(juce::Array<Parameter *> parameters);
	virtual ~FloatParameterLabelUI() {};

	FloatParameter* floatParam;

	juce::Label nameLabel;
	juce::Label valueLabel;

	juce::String prefix;
	juce::String suffix;

	float maxFontHeight;
    
	const float pixelsPerUnit = 10; //1 = 10pixel
	float valueAtMouseDown;
	float valueOffsetSinceMouseDown;
	int lastMouseX;

	bool autoSize;
	void setAutoSize(bool value);

	void setPrefix(const juce::String &_prefix);
	void setSuffix(const juce::String &_suffix);

	virtual void updateValueFromLabel();
	virtual void updateTooltip() override;

	//void paint(Graphics &g) override;
	void resized() override;

	void mouseDownInternal(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUpInternal(const juce::MouseEvent &e) override;

	virtual void focusGained(FocusChangeType cause) override;

	void updateUIParamsInternal() override;

	virtual void handlePaintTimerInternal() override;

protected:
	virtual juce::String getValueString(const juce::var &val) const;

	virtual void valueChanged(const juce::var & v) override;
	virtual void labelTextChanged(juce::Label* labelThatHasChanged) override;
	virtual void editorShown(juce::Label* label, juce::TextEditor&) override;
	virtual void editorHidden(juce::Label* label, juce::TextEditor&) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatParameterLabelUI)

};

class TimeLabel :
	public FloatParameterLabelUI
{
public:
	TimeLabel(juce::Array<Parameter *> parameters);
	~TimeLabel();

	bool showStepsMode;
	void setShowStepsMode(bool stepsMode);

protected:

	void valueChanged(const juce::var &) override;
	virtual void editorShown(juce::Label* label, juce::TextEditor&) override;
	void labelTextChanged(juce::Label * l) override;
	virtual void updateValueFromLabel() override;

	juce::String getValueString(const juce::var &val) const override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeLabel)
};