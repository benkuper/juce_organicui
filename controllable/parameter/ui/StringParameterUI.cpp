#include "StringParameterUI.h"
/*
  ==============================================================================

    StringParameterUI.cpp
    Created: 9 Mar 2016 12:29:57am
    Author:  bkupe

  ==============================================================================
*/



StringParameterUI::StringParameterUI(Parameter * p) :
    ParameterUI(p),
	maxFontHeight(12),
	autoSize(false)
{
	showEditWindowOnDoubleClick = false;
	showEditWindowOnRightClick = false;

    addAndMakeVisible(valueLabel);

    valueLabel.setJustificationType(Justification::topLeft);
    valueLabel.setText(parameter->value,NotificationType::dontSendNotification);
    valueLabel.setEnabled(parameter->isEditable && !forceFeedbackOnly);
	valueLabel.setEditable(false,parameter->isEditable && !forceFeedbackOnly);
	valueLabel.addListener(this);

	valueLabel.setColour(valueLabel.backgroundWhenEditingColourId, Colours::black);
	valueLabel.setColour(valueLabel.textWhenEditingColourId, Colours::white);
	valueLabel.setColour(valueLabel.textColourId, parameter->isEditable ? TEXT_COLOR : TEXT_COLOR.withAlpha(.6f));
	valueLabel.setColour(CaretComponent::caretColourId, Colours::orange);
	
	valueLabel.addMouseListener(this, false);

    setSize(200, 20);//default size
}

void StringParameterUI::setAutoSize(bool value)
{
	autoSize = value;
	valueChanged(parameter->getValue());
}

bool StringParameterUI::isEditing()
{
	return valueLabel.isBeingEdited();
}

void StringParameterUI::setPrefix(const String & _prefix)
{
	if (prefix == _prefix) return;
	prefix = _prefix;
	valueChanged(parameter->stringValue());
}

void StringParameterUI::setSuffix(const String & _suffix)
{
	if (suffix == _suffix) return;
	suffix = _suffix;
	valueChanged(parameter->stringValue());
}


void StringParameterUI::setOpaqueBackground(bool value)
{
	ParameterUI::setOpaqueBackground(value);
	valueLabel.setColour(valueLabel.backgroundColourId, opaqueBackground ? (parameter->isEditable ? BG_COLOR.darker(.1f).withAlpha(.7f):BG_COLOR.brighter(.2f)) : Colours::transparentBlack);
	valueLabel.setColour(valueLabel.outlineColourId, opaqueBackground ? BG_COLOR.brighter(.1f):Colours::transparentWhite);
	
}

void StringParameterUI::setForceFeedbackOnlyInternal()
{
	ParameterUI::setForceFeedbackOnlyInternal();
	valueLabel.setEditable(false, parameter->isEditable && !forceFeedbackOnly);
	valueLabel.setEnabled(parameter->isEditable && !forceFeedbackOnly);
	setOpaqueBackground(opaqueBackground); //force refresh color
}

void StringParameterUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	resizedInternal(r);
	valueLabel.setBounds(r);
	valueLabel.setFont(valueLabel.getFont().withHeight(jmin<float>((float)r.getHeight(), maxFontHeight)));
}

void StringParameterUI::resizedInternal(juce::Rectangle<int>& r)
{
	//to be overriden
}

void StringParameterUI::valueChanged(const var & v)
{
    valueLabel.setText(prefix+parameter->stringValue()+suffix,NotificationType::dontSendNotification);

	if (autoSize)
	{
		int valueLabelWidth = valueLabel.getFont().getStringWidth(valueLabel.getText());
		int tw = valueLabelWidth;
		setSize(tw + 10,(int)valueLabel.getFont().getHeight());
	}

}

void StringParameterUI::labelTextChanged(Label *)
{
	//String  originalString = valueLabel.getText().substring(prefix.length(), valueLabel.getText().length() - suffix.length());
	parameter->setUndoableValue(parameter->stringValue(),valueLabel.getText());
}

StringParameterFileUI::StringParameterFileUI(Parameter * p) :
	StringParameterUI(p),
	fp((FileParameter *)p),
	browseBT("Browse...")
{
	relativeBT = AssetManager::getInstance()->getToggleBTImage(AssetManager::getInstance()->getRelativeImage());
	relativeBT->setToggleState(fp->forceRelativePath, dontSendNotification);
	browseBT.addListener(this);
	
	relativeBT->addListener(this); 
	
	addAndMakeVisible(&browseBT);
	addAndMakeVisible(relativeBT);
}

StringParameterFileUI::~StringParameterFileUI()
{
}

void StringParameterFileUI::resizedInternal(juce::Rectangle<int> &r)
{
	relativeBT->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);
	browseBT.setBounds(r.removeFromRight(60));
}

void StringParameterFileUI::buttonClicked(Button * b)
{
	if (b == &browseBT)
	{
		FileChooser chooser("Select a file");
		bool result = chooser.browseForFileToOpen();
		if (result) parameter->setUndoableValue(parameter->stringValue(),chooser.getResult().getFullPathName());
	} else if (b == relativeBT)
	{
		fp->setForceRelativePath(!fp->forceRelativePath);
	}
}
