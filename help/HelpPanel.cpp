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
	helpLabel.setFont(14);
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

