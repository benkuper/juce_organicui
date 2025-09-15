/*
  ==============================================================================

	ColorParameterUI.h
	Created: 11 Apr 2017 10:42:03am
	Author:  Ben

  ==============================================================================
*/

#pragma once

class OrganicColorPicker :
	public juce::Component,
	public Parameter::AsyncListener
{
public:
	OrganicColorPicker(ColorParameter* colorParam);
	~OrganicColorPicker();

	ColorParameter* colorParam;
	juce::WeakReference<Parameter> paramRef;

	FloatParameter r;
	std::unique_ptr<FloatSliderUI> rUI;
	FloatParameter g;
	std::unique_ptr<FloatSliderUI> gUI;
	FloatParameter b;
	std::unique_ptr<FloatSliderUI> bUI;

	FloatParameter h;
	std::unique_ptr<FloatSliderUI> hUI;
	FloatParameter s;
	std::unique_ptr<FloatSliderUI> sUI;
	FloatParameter bri;
	std::unique_ptr<FloatSliderUI> briUI;

	FloatParameter a;
	std::unique_ptr<FloatSliderUI> aUI;

	StringParameter hex;
	std::unique_ptr<StringParameterUI> hexUI;

	juce::Colour lastColor;
	juce::Rectangle<int> hueSatRect;
	juce::Image hueSatImage;

	bool isUpdatingColor;
	bool isDraggingHueSat;

	class HueSatHandle :
		public juce::Component
	{
	public:
		HueSatHandle(OrganicColorPicker* picker);
		~HueSatHandle();

		OrganicColorPicker* picker;

		void paint(juce::Graphics& g) override;
		void resized() override;
		void mouseDown(const juce::MouseEvent& e) override;
		void mouseDrag(const juce::MouseEvent& e) override;
		void mouseUp(const juce::MouseEvent& e) override;
	};

	std::unique_ptr<HueSatHandle> hueSatHandle;

	void paint(juce::Graphics& _g) override;
	void resized() override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;

	void regenerateHueSatImage();
	void updateHueSat();

	void setEditingColor(const juce::Colour& c, bool setRGB = true, bool setHSV = true, bool setParam = true);
	void setUndoableParam();

	void updateFromRGB(float r, float g, float b);
	void updateFromHSV(float h, float s, float v);
	void updateFromHex(juce::String hex);
	void updateFromAlpha(float alpha);
	void updateHueSatHandle();
	void updateFromParameter();

	void newMessage(const Parameter::ParameterEvent& e) override;
};

class ColorParameterUI :
	public ParameterUI,
	public juce::ComponentListener
{

public:
	ColorParameterUI(juce::Array<ColorParameter*> parameters);
	~ColorParameterUI();

	juce::Array<ColorParameter*> colorParams;
	juce::var valueOnEditorOpen;
	ColorParameter* colorParam;
	juce::CallOutBox* colorEditor;

	bool dispatchOnDoubleClick;
	bool dispatchOnSingleClick;

	void paint(juce::Graphics& g) override;
	void resized() override;
	void mouseDownInternal(const juce::MouseEvent& e) override;

	void showEditWindowInternal() override;
	void showEditRangeWindowInternal() override;

	void componentBeingDeleted(juce::Component&) override;

protected:
	// Inherited via ChangeListener
	void valueChanged(const juce::var&) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorParameterUI)

};