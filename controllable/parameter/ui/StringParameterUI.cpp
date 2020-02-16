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
	valueLabel(p->niceName+"_ValueLabel"),
	maxFontHeight(16),
	autoSize(false)
{
	showEditWindowOnDoubleClick = false;

    addAndMakeVisible(&valueLabel);

    valueLabel.setJustificationType(Justification::topLeft);
    valueLabel.setText(parameter->getValue(),NotificationType::dontSendNotification);
	valueLabel.addListener(this);

	valueLabel.setColour(valueLabel.backgroundWhenEditingColourId, Colours::black);
	valueLabel.setColour(valueLabel.textWhenEditingColourId, Colours::white);
	valueLabel.setColour(CaretComponent::caretColourId, Colours::orange);
	valueLabel.setColour(valueLabel.textColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));
	valueLabel.addMouseListener(this, false);

	ParameterUI::setNextFocusOrder(&valueLabel);

	feedbackStateChanged();

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue()+4);//default size
}

StringParameterUI::~StringParameterUI()
{
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
	valueLabel.setColour(valueLabel.backgroundColourId, useCustomBGColor?customBGColor:(opaqueBackground ? (!controllable->isControllableFeedbackOnly ? BG_COLOR.darker(.1f).withAlpha(.7f):BG_COLOR.brighter(.2f)) : Colours::transparentBlack));
	valueLabel.setColour(valueLabel.outlineColourId, useCustomBGColor ? customBGColor.brighter():(opaqueBackground ? BG_COLOR.brighter(.1f):Colours::transparentWhite));
	valueLabel.setColour(valueLabel.textColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));
	//valueLabel.setColour(valueLabel.textColourId, !controllable->isControllableFeedbackOnly ? TEXT_COLOR : TEXT_COLOR.withAlpha(.6f));
}

void StringParameterUI::feedbackStateChanged()
{
	valueLabel.setEditable(!controllable->isControllableFeedbackOnly);
	valueLabel.setEnabled(!controllable->isControllableFeedbackOnly);
	valueLabel.setColour(valueLabel.textColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));
	setOpaqueBackground(opaqueBackground); //force refresh color
}

void StringParameterUI::updateTooltip()
{
	ParameterUI::updateTooltip();
	valueLabel.setTooltip(tooltip);
}

void StringParameterUI::paint(Graphics& g)
{
	ParameterUI::paint(g);

	if (showLabel)
	{
		Rectangle<int> r = getLocalBounds();
		g.setFont(jlimit(12, 40, jmin(r.getHeight()-2, r.getWidth()) - 16));
		r = r.removeFromLeft(jmin(g.getCurrentFont().getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName)+10, r.getWidth() - 60));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, r, Justification::centred, 1);
	}
}

void StringParameterUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	if (showLabel)
	{
		Font font(jlimit(12, 40, jmin(r.getHeight()-2, r.getWidth()) - 16));
		r.removeFromLeft(jmin(font.getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName)+10, r.getWidth() - 60));
		r.removeFromLeft(2);
		valueLabel.setJustificationType(Justification::centred);
	}

	resizedInternal(r);

	valueLabel.setBounds(r);
	valueLabel.setFont(valueLabel.getFont().withHeight(jmin<float>((float)r.getHeight()-4, maxFontHeight)));
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
	if (stringParam->autoTrim) valueLabel.setText(valueLabel.getText().trim(), dontSendNotification);
	parameter->setUndoableValue(parameter->stringValue(),valueLabel.getText());
}

StringParameterFileUI::StringParameterFileUI(Parameter * p) :
	StringParameterUI(p),
	fp((FileParameter *)p),
	browseBT("Browse...")
{
	browseBT.addListener(this);
	browseBT.setEnabled(isInteractable());
	addAndMakeVisible(&browseBT);

	if (!fp->forceRelativePath)
	{
		relativeBT.reset(AssetManager::getInstance()->getToggleBTImage(AssetManager::getInstance()->getRelativeImage()));
		relativeBT->addListener(this);
		relativeBT->setToggleState(fp->forceRelativePath, dontSendNotification);
		relativeBT->setEnabled(isInteractable());
		addAndMakeVisible(relativeBT.get());
	}

}

StringParameterFileUI::~StringParameterFileUI()
{
}

void StringParameterFileUI::resizedInternal(juce::Rectangle<int> &r)
{
	if (relativeBT != nullptr)
	{
		relativeBT->setBounds(r.removeFromRight(r.getHeight()));
		r.removeFromRight(2);
	}

	browseBT.setBounds(r.removeFromRight(60));
}

void StringParameterFileUI::feedbackStateChanged()
{
	StringParameterUI::feedbackStateChanged();
	browseBT.setEnabled(isInteractable());
	if (relativeBT != nullptr) relativeBT->setEnabled(isInteractable());
}

void StringParameterFileUI::buttonClicked(Button * b)
{
	if (b == &browseBT)
	{
		Component::BailOutChecker checker(this); 
		FileChooser chooser("Select a file", File(fp->getAbsolutePath()), fp->fileTypeFilter);
        bool result;
        FileParameter * fp = (FileParameter *) parameter.get();
        
        if (fp->directoryMode) result = chooser.browseForDirectory();
        else result = chooser.browseForFileToOpen();
        
		if (checker.shouldBailOut()) return;

		if (parameter.wasObjectDeleted()) return;
		if (result && parameter != nullptr)
		{
			parameter->setUndoableValue(parameter->stringValue(), chooser.getResult().getFullPathName());
		}
	} else if (relativeBT != nullptr && b == relativeBT.get())
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
	ParameterUI::feedbackStateChanged();
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
