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
	public Label::Listener
{
public:
	FloatParameterLabelUI(Array<Parameter *> parameters);
	virtual ~FloatParameterLabelUI() {};

	Label nameLabel;
	Label valueLabel;

	String prefix;
	String suffix;

	float maxFontHeight;
    
	const float pixelsPerUnit = 10; //1 = 10pixel
	float valueAtMouseDown;
	float valueOffsetSinceMouseDown;
	int lastMouseX;

	bool autoSize;
	void setAutoSize(bool value);

	void setPrefix(const String &_prefix);
	void setSuffix(const String &_suffix);

	virtual void updateLabelFromValue();
	virtual void updateTooltip() override;

	//void paint(Graphics &g) override;
	void resized() override;

	void mouseDownInternal(const MouseEvent &e) override;
	void mouseDrag(const MouseEvent &e) override;
	void mouseUpInternal(const MouseEvent &e) override;

	virtual void focusGained(FocusChangeType cause) override;

	void updateUIParamsInternal() override;

	virtual void handlePaintTimerInternal() override;

protected:
	virtual String getValueString(const var &val) const;

	virtual void valueChanged(const var & v) override;
	virtual void labelTextChanged(Label * labelThatHasChanged) override;
	virtual void editorShown(Label* label, TextEditor&) override;
	virtual void editorHidden(Label* label, TextEditor&) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatParameterLabelUI)

};

class TimeLabel :
	public FloatParameterLabelUI
{
public:
	TimeLabel(Array<Parameter *> parameters);
	~TimeLabel();

	bool showStepsMode;
	void setShowStepsMode(bool stepsMode);

protected:

	void valueChanged(const var &) override;
	virtual void editorShown(Label* label, TextEditor&) override;
	void labelTextChanged(Label * l) override;

	String getValueString(const var &val) const override;
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeLabel)
};