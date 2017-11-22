/*
  ==============================================================================

    HelpPanel.h
    Created: 22 Nov 2017 11:31:53am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class HelpPanel :
	public ShapeShifterContentComponent,
	public HelpBox::HelpListener
{
public:
	HelpPanel(const String &name);
	~HelpPanel();

	Label helpLabel;

	void resized() override;

	void helpContentChanged() override;

	static HelpPanel * create(const String &contentName) { return new HelpPanel(contentName); }
};