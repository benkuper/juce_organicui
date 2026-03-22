/*
  ==============================================================================

	StringParameterUI.cpp
	Created: 9 Mar 2016 12:29:57am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"


StringParameterUI::StringParameterUI(Array<StringParameter*> parameters) :
	ParameterUI(Inspectable::getArrayAs<StringParameter, Parameter>(parameters)),
	stringParams(parameters),
	stringParam(parameters[0]),
	valueLabel(parameters[0]->niceName + "_ValueLabel"),
	maxFontHeight(16),
	autoSize(false)
{
	showEditWindowOnDoubleClick = false;

	addAndMakeVisible(&valueLabel);

	valueLabel.setJustificationType(Justification::topLeft);
	valueLabel.setText(parameter->getValue(), NotificationType::dontSendNotification);
	valueLabel.addListener(this);
	valueLabel.addMouseListener(this, false);

	ParameterUI::setNextFocusOrder(&valueLabel);

	updateUIParams();

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size
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

void StringParameterUI::updateTooltip()
{
	ParameterUI::updateTooltip();
	valueLabel.setTooltip(tooltip);
}

void StringParameterUI::updateUIParamsInternal()
{
	valueLabel.setEditable(!controllable->isControllableFeedbackOnly);
	valueLabel.setEnabled(!controllable->isControllableFeedbackOnly);
	valueLabel.setColour(valueLabel.backgroundColourId, useCustomBGColor ? customBGColor : (opaqueBackground ? (!controllable->isControllableFeedbackOnly ? BG_COLOR.darker(.1f).withAlpha(.7f) : BG_COLOR.darker(.1f).withAlpha(.4f)) : Colours::transparentBlack));
	valueLabel.setColour(valueLabel.outlineColourId, useCustomBGColor ? customBGColor.brighter() : (opaqueBackground ? BG_COLOR.brighter(.1f) : Colours::transparentWhite));
	valueLabel.setColour(valueLabel.textColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));
	valueLabel.setColour(CaretComponent::caretColourId, Colours::orange);

	if (customTextSize > 0) valueLabel.setFont(customTextSize);
	else valueLabel.setFont(Font());
}

void StringParameterUI::paint(Graphics& g)
{
	ParameterUI::paint(g);

	if (showLabel)
	{
		juce::Rectangle<int> r = getLocalBounds();
		float fontHeight = customTextSize > 0 ? customTextSize : jlimit(12, 40, jmin(r.getHeight() - 2, r.getWidth()) - 16);
		g.setFont(fontHeight);
		r = r.removeFromLeft(jmin(g.getCurrentFont().getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 60));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, r, Justification::centred, 1);
	}
}

void StringParameterUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	float fontHeight = customTextSize > 0 ? customTextSize : jlimit(12, 40, jmin(r.getHeight() - 2, r.getWidth()) - 16);

	if (showLabel)
	{
		Font font(fontHeight);
		r.removeFromLeft(jmin(font.getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 60));
		r.removeFromLeft(2);
		valueLabel.setJustificationType(Justification::centred);
	}

	resizedInternal(r);

	valueLabel.setBounds(r);
	//valueLabel.setFont(valueLabel.getFont().withHeight(jmin<float>((float)r.getHeight() - 4, maxFontHeight)));
}

void StringParameterUI::valueChanged(const var& v)
{
	valueLabel.setText(stringParam->prefix + parameter->stringValue() + stringParam->suffix, NotificationType::dontSendNotification);

	if (autoSize)
	{
		int valueLabelWidth = valueLabel.getFont().getStringWidth(valueLabel.getText());
		int tw = valueLabelWidth;
		setSize(tw + 10, (int)valueLabel.getFont().getHeight());
	}

}

void StringParameterUI::labelTextChanged(Label*)
{
	//String  originalString = valueLabel.getText().substring(prefix.length(), valueLabel.getText().length() - suffix.length());
	if (stringParam->autoTrim) valueLabel.setText(valueLabel.getText().trim(), dontSendNotification);
	parameter->setUndoableValue(parameter->stringValue(), valueLabel.getText());
}


File StringParameterFileUI::lastSearchedFolder = File();

StringParameterFileUI::StringParameterFileUI(Array<StringParameter*> parameters) :
	StringParameterUI(parameters),
	fps(Inspectable::getArrayAs<StringParameter, FileParameter>(parameters)),
	fp(dynamic_cast<FileParameter*>(parameters[0])),
	browseBT("Browse...")
{
	browseBT.addListener(this);
	browseBT.setEnabled(isInteractable());
	addAndMakeVisible(&browseBT);

	explorerBT.reset(AssetManager::getInstance()->getSetupBTImage(AssetManager::getInstance()->explorerImage));
	addAndMakeVisible(explorerBT.get());
	explorerBT->addListener(this);

}

StringParameterFileUI::~StringParameterFileUI()
{
}

void StringParameterFileUI::addPopupMenuItemsInternal(PopupMenu* p)
{
	StringParameterUI::addPopupMenuItemsInternal(p);
	p->addSeparator();
	p->addItem(30, "Force Relative Path", true, fp->forceRelativePath);
	p->addItem(31, "Force Absolute Path", true, fp->forceAbsolutePath);
	p->addItem(32, "Show in Explorer");
}

void StringParameterFileUI::handleMenuSelectedID(int result)
{
	StringParameterUI::handleMenuSelectedID(result);
	switch (result)
	{
	case 30:
		fp->setForceRelativePath(!fp->forceRelativePath);
		break;

	case 31:
		fp->setForceAbsolutePath(!fp->forceAbsolutePath);
		break;

	case 32:
		if (fp->getFile().exists()) fp->getFile().revealToUser();
		break;
	}
}

void StringParameterFileUI::resizedInternal(juce::Rectangle<int>& r)
{
	explorerBT->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);

	browseBT.setBounds(r.removeFromRight(60));
}

void StringParameterFileUI::feedbackStateChanged()
{
	StringParameterUI::feedbackStateChanged();
	browseBT.setEnabled(isInteractable());
	//explorerBT.setEnabled(isInteractable());
}

void StringParameterFileUI::buttonClicked(Button* b)
{
	if (b == &browseBT)
	{
		File of = fp->getFile().exists() ? fp->getFile().getParentDirectory() : (lastSearchedFolder == File() ? Engine::mainEngine->getFile().getParentDirectory() : lastSearchedFolder);

		FileChooser* chooser(new FileChooser("Select a file", of, fp->fileTypeFilter));

		int openFlags = fp->saveMode ? FileBrowserComponent::saveMode : FileBrowserComponent::openMode;
		if (fp->directoryMode) openFlags = openFlags | FileBrowserComponent::canSelectDirectories;
		else openFlags = openFlags | FileBrowserComponent::FileChooserFlags::canSelectFiles;

		chooser->launchAsync(openFlags, [this](const FileChooser& fc)
			{
				File f = fc.getResult();
				delete& fc;

				if (!f.exists() && !fp->saveMode) return;

				lastSearchedFolder = f.getParentDirectory();

				Component::BailOutChecker checker(this);
				if (checker.shouldBailOut()) return;

				if (parameter.wasObjectDeleted()) return;
				if (parameter != nullptr)
				{
					parameter->setUndoableValue(parameter->stringValue(), f.getFullPathName());
				}

			}
		);
	}
	else if (b == explorerBT.get())
	{
		if (fp->getFile().existsAsFile()) fp->getFile().revealToUser();
	}
}

StringParameterTextUI::StringParameterTextUI(Array<StringParameter*> parameters) :
	ParameterUI(Inspectable::getArrayAs<StringParameter, Parameter>(parameters)),
	stringParams(parameters),
	stringParam(parameters[0])
{
	editor.setColour(editor.backgroundColourId, Colours::black);
	editor.setColour(CaretComponent::caretColourId, Colours::orange);
	editor.setColour(editor.textColourId, isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f));
	editor.setMultiLine(stringParam->multiline);
	editor.setText(stringParam->stringValue(), false);
	editor.setReturnKeyStartsNewLine(stringParam->multiline);
	editor.addListener(this);

	addAndMakeVisible(&editor);
	setSize(100, stringParam->multiline ? 60 : 16);
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

void StringParameterTextUI::valueChanged(const var& v)
{
	editor.setText(stringParam->stringValue(), false);
}

void StringParameterTextUI::textEditorTextChanged(TextEditor&)
{
	stringParam->setValue(editor.getText());
}

void StringParameterTextUI::textEditorFocusLost(TextEditor&)
{

}

void StringParameterTextUI::textEditorReturnKeyPressed(TextEditor&)
{
}
