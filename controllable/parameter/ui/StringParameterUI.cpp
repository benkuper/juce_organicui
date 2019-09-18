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
	stringParam((StringParameter *)p),
	maxFontHeight(14),
	autoSize(false)
{
	showEditWindowOnDoubleClick = false;

    addAndMakeVisible(valueLabel);

    valueLabel.setJustificationType(Justification::topLeft);
    valueLabel.setText(parameter->getValue(),NotificationType::dontSendNotification);
	valueLabel.addListener(this);

	valueLabel.setColour(valueLabel.backgroundWhenEditingColourId, Colours::black);
	valueLabel.setColour(valueLabel.textWhenEditingColourId, Colours::white);
	valueLabel.setColour(CaretComponent::caretColourId, Colours::orange);
	valueLabel.setColour(valueLabel.textColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));

	valueLabel.addMouseListener(this, false);
	ParameterUI::setNextFocusOrder(&valueLabel);

	feedbackStateChanged();

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

void StringParameterUI::setOpaqueBackground(bool value)
{
	ParameterUI::setOpaqueBackground(value);
	valueLabel.setColour(valueLabel.backgroundColourId, opaqueBackground ? (!controllable->isControllableFeedbackOnly ? BG_COLOR.darker(.1f).withAlpha(.7f):BG_COLOR.brighter(.2f)) : Colours::transparentBlack);
	valueLabel.setColour(valueLabel.outlineColourId, opaqueBackground ? BG_COLOR.brighter(.1f):Colours::transparentWhite);
	valueLabel.setColour(valueLabel.textColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));
	//valueLabel.setColour(valueLabel.textColourId, !controllable->isControllableFeedbackOnly ? TEXT_COLOR : TEXT_COLOR.withAlpha(.6f));
}

void StringParameterUI::feedbackStateChanged()
{
	valueLabel.setEditable(!controllable->isControllableFeedbackOnly);
	valueLabel.setEnabled(!controllable->isControllableFeedbackOnly);
	valueLabel.setColour(valueLabel.textColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));
	setOpaqueBackground(opaqueBackground); //force refresh color
}

void StringParameterUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	resizedInternal(r);
	valueLabel.setBounds(r);
	valueLabel.setFont(valueLabel.getFont().withHeight(jmin<float>((float)r.getHeight(), maxFontHeight)));
}

void StringParameterUI::valueChanged(const var & v)
{
    valueLabel.setText(stringParam->prefix+parameter->stringValue()+stringParam->suffix,NotificationType::dontSendNotification);

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
	relativeBT.reset(AssetManager::getInstance()->getToggleBTImage(AssetManager::getInstance()->getRelativeImage()));
	relativeBT->setToggleState(fp->forceRelativePath, dontSendNotification);
	browseBT.addListener(this);
	
	relativeBT->addListener(this); 
	
	addAndMakeVisible(&browseBT);
	addAndMakeVisible(relativeBT.get());
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
		FileChooser chooser("Select a file", File(fp->getAbsolutePath()), fp->fileTypeFilter);
		bool result = chooser.browseForFileToOpen();
		if (result && parameter != nullptr && !parameter.wasObjectDeleted())
		{
			parameter->setUndoableValue(parameter->stringValue(), chooser.getResult().getFullPathName());
		}
	} else if (b == relativeBT.get())
	{
		fp->setForceRelativePath(!fp->forceRelativePath);
	}
}

StringParameterTextUI::StringParameterTextUI(Parameter * p) :
	ParameterUI(p),
	stringParam((StringParameter *)p)
{
	editor.setColour(editor.backgroundColourId, Colours::black);
	editor.setColour(CaretComponent::caretColourId, Colours::orange);
	editor.setColour(editor.textColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));
	editor.setMultiLine(stringParam->multiline);
	editor.setText(stringParam->stringValue(), false);
	editor.setReturnKeyStartsNewLine(stringParam->multiline);
	editor.addListener(this);
	
	addAndMakeVisible(&editor);
	setSize(100, stringParam->multiline?60:16);
}

void StringParameterTextUI::feedbackStateChanged()
{
	editor.setColour(editor.textColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));
	editor.setEnabled(isInteractable());
}

void StringParameterTextUI::resized()
{
	editor.setBounds(getLocalBounds());
}

void StringParameterTextUI::valueChanged(const var & v)
{
	editor.setText(stringParam->stringValue(), false);
}

void StringParameterTextUI::textEditorTextChanged(TextEditor &)
{
	stringParam->setValue(editor.getText());
}

void StringParameterTextUI::textEditorFocusLost(TextEditor &)
{
	
}

void StringParameterTextUI::textEditorReturnKeyPressed(TextEditor &)
{
}
