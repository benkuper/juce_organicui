#include "HelpPanel.h"
/*
  ==============================================================================

    HelpPanel.cpp
    Created: 22 Nov 2017 11:31:53am
    Author:  Ben

  ==============================================================================
*/


HelpPanel::HelpPanel(const String &name) :
	ShapeShifterContentComponent(name),
	helpLabel("HelpLabel")
{
	helpLabel.setJustificationType(Justification::topLeft);
	helpLabel.setFont(FontOptions(14));

	if (GlobalSettings::getInstance()->helpLanguage->getValueData().toString() == "cn")
	{
		helpLabel.setFont(FontOptions(16));

		Font font = helpLabel.getFont();
		String fontName;
#if JUCE_WINDOWS
		fontName = "Microsoft YaHei";
#elif JUCE_MAC
		fontName = "PingFang SC";
#else
		fontName = "WenQuanYi Zen Hei";
#endif
		if (fontName.isNotEmpty()) font.setTypefaceName(fontName);

		helpLabel.setFont(font);
	}


	helpLabel.setColour(helpLabel.textColourId, PANEL_COLOR.brighter(.4f));
	addAndMakeVisible(&helpLabel);
	HelpBox::getInstance()->addHelpListener(this);
}

HelpPanel::~HelpPanel()
{
	if(HelpBox::getInstanceWithoutCreating() != nullptr) HelpBox::getInstance()->removeHelpListener(this);
}

void HelpPanel::resized()
{
	helpLabel.setBounds(getLocalBounds().reduced(5));
}

void HelpPanel::helpContentChanged()
{
	helpLabel.setText(HelpBox::getInstance()->getCurrentData(),dontSendNotification);
}

