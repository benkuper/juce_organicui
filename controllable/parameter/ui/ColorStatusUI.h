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
	ColorStatusUI(Array<Parameter*> parameters, bool isCircle = true);
	~ColorStatusUI();

	bool isCircle;
	bool momentaryMode;

	void paint(Graphics& g) override;

	Colour getCurrentColor() const;

	void mouseDownInternal(const MouseEvent& e) override;
	void mouseUpInternal(const MouseEvent& e) override;

	class ColorOptionManager :
		public Component,
		public Label::Listener,
		public Parameter::ParameterListener
	{
	public:
		ColorOptionManager(Parameter* parameter);
		~ColorOptionManager();


		Parameter* parameter;
		Viewport viewport;
		Component container;


		class ColorOptionUI :
			public Component,
			public ChangeListener
		{
		public:
			ColorOptionUI(Parameter* p, const var& key);

			Parameter* parameter;

			Label keyLabel;
			ColorParameter cp;
			std::unique_ptr<ColorParameterUI> cpui;

			void resized() override;
			virtual void changeListenerCallback(ChangeBroadcaster* source) override;
		};

		OwnedArray<ColorOptionUI> optionsUI;


		void setImages(Image onImage, Image offImage = Image());

		

		void addOptionUI(const var& key);

		void paint(Graphics& g) override;
		void resized() override;

		void labelTextChanged(Label* l) override;
		void parameterValueChanged(Parameter* p) override;

		void updateColorOptions();

		static void show(Parameter* p, Component* c);
	};



protected:
	// Inherited via ChangeListener
	void valueChanged(const var&) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorStatusUI)
};