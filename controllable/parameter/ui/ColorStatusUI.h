/*
  ==============================================================================

	ColorParameterUI.h
	Created: 11 Apr 2017 10:42:03am
	Author:  Ben

  ==============================================================================
*/

#pragma once

class ColorStatusUI :
	public ParameterUI
{

public:
	ColorStatusUI(juce::Array<Parameter*> parameters, bool isCircle = true);
	~ColorStatusUI();

	bool isCircle;
	bool momentaryMode;

	void paint(juce::Graphics& g) override;

	juce::Colour getCurrentColor() const;

	void mouseDownInternal(const juce::MouseEvent& e) override;
	void mouseUpInternal(const juce::MouseEvent& e) override;

	class ColorOptionManager :
		public juce::Component,
		public juce::Label::Listener,
		public Parameter::ParameterListener
	{
	public:
		ColorOptionManager(Parameter* parameter);
		~ColorOptionManager();


		Parameter* parameter;
		juce::Viewport viewport;
		juce::Component container;


		class ColorOptionUI :
			public juce::Component,
			public juce::ChangeListener
		{
		public:
			ColorOptionUI(Parameter* p, const juce::var& key);

			Parameter* parameter;

			juce::Label keyLabel;
			ColorParameter cp;
			std::unique_ptr<ColorParameterUI> cpui;

			void resized() override;
			virtual void changeListenerCallback(juce::ChangeBroadcaster* source) override;
		};

		juce::OwnedArray<ColorOptionUI> optionsUI;


		void setImages(juce::Image onImage, juce::Image offImage = juce::Image());

		

		void addOptionUI(const juce::var& key);

		void paint(juce::Graphics& g) override;
		void resized() override;

		void labelTextChanged(juce::Label* l) override;
		void parameterValueChanged(Parameter* p) override;

		void updateColorOptions();

		static void show(Parameter* p, juce::Component* c);
	};



protected:
	// Inherited via ChangeListener
	void valueChanged(const juce::var&) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorStatusUI)
};