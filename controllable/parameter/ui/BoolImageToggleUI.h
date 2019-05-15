/*
  ==============================================================================

    BoolImageToggleUI.h
    Created: 4 Jan 2017 1:33:12pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class BoolImageToggleUI :
	public ParameterUI,
	public Button::Listener
{
public:
	BoolImageToggleUI(ImageButton *, BoolParameter *);
	~BoolImageToggleUI();
	
	BoolParameter * boolParam;
	std::unique_ptr<ImageButton> bt;

	void setTooltip(const String &value) override;

	void resized() override;

	void valueChanged(const var &) override;

	void buttonClicked(Button *) override;
};