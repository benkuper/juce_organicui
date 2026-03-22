/*
  ==============================================================================

	BoolToggleUI.cpp
	Created: 8 Mar 2016 3:47:01pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

BoolToggleUI::BoolToggleUI(Array<BoolParameter*> parameters, Image _onImage, Image _offImage) :
	ParameterUI(Inspectable::getArrayAs<BoolParameter, Parameter>(parameters), ORGANICUI_DEFAULT_TIMER),
	momentaryMode(false)
{
	usingCustomImages = _onImage.isValid();


	if (usingCustomImages)
	{
		setImages(_onImage, _offImage);
		showLabel = false;
	}
	else
	{
		updateUIParams();
	}


	showEditWindowOnDoubleClick = false;

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size

}

BoolToggleUI::~BoolToggleUI()
{

}

void BoolToggleUI::setImages(Image _onImage, Image _offImage)
{
	onImage = _onImage;
	offImage = _offImage;

	if (onImage.isValid() && !offImage.isValid()) {
		offImage = onImage.createCopy();
		offImage.desaturate();
	}
}

void BoolToggleUI::paint(Graphics& g)
{
	// we are on component deletion
	if (shouldBailOut())return;

	bool valCheck = parameter->boolValue();
	Image m = valCheck ? onImage : offImage;

	juce::Rectangle<int> r = getLocalBounds();
	//g.setColour(Colours::white.withAlpha(isMouseOver() ? 1 : .8f));

	juce::Rectangle<int> cr;
	float labelWidth = 0;

	if (showLabel)
	{
		int fontSize = customTextSize > 0 ? customTextSize : jlimit(12, 40, jmin(r.getHeight(), r.getWidth()) - 16);
		g.setFont(fontSize);
		labelWidth = g.getCurrentFont().getStringWidth(parameter->niceName) + 10;
		/*if (r.getHeight() > r.getWidth())
		{
			cr = r.removeFromRight(jmin<float>(r.getHeight(), r.getWidth() - labelWidth));
			cr = cr.withSizeKeepingCentre(cr.getWidth(), cr.getWidth());
		}
		else*/
		{
			cr = r.removeFromRight(getHeight());
		}

		r.removeFromRight(2);
	}
	else
	{
		cr = r.removeFromLeft(r.getHeight());
	}

	if (showLabel)
	{
		g.setColour(useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : FEEDBACK_COLOR));
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, r, Justification::left, 1);
	}

	if (m.isValid())
	{
		g.setOpacity(isMouseOverOrDragging() ? 1.0f : .8f);
		g.drawImage(m, cr.toFloat(), RectanglePlacement::stretchToFit);
	}
}

void BoolToggleUI::mouseDownInternal(const MouseEvent& e)
{
	if (!isInteractable()) return;
	if (e.mods.isLeftButtonDown())
	{
		if (e.mods.isAltDown() || momentaryMode) parameter->setValue(!parameter->boolValue());
		else parameter->setUndoableValue(parameter->boolValue(), !parameter->boolValue()); //only undoable when from left button, real toggle behaviour
	}
}

void BoolToggleUI::mouseUpInternal(const MouseEvent& e)
{
	if (!isInteractable()) return;
	if (e.mods.isLeftButtonDown())
	{
		if (e.mods.isAltDown() || momentaryMode) parameter->setValue(!parameter->boolValue());
	}
}

void BoolToggleUI::updateUIParamsInternal()
{
	ParameterUI::updateUIParamsInternal();

	if (!usingCustomImages)
	{
		if (!isInteractable())
		{
			setImages(ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_on_readonly_png, OrganicUIBinaryData::checkbox_on_readonly_pngSize), ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_off_readonly_png, OrganicUIBinaryData::checkbox_off_readonly_pngSize));
		}
		else
		{
			setImages(ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_on_png, OrganicUIBinaryData::checkbox_on_pngSize), ImageCache::getFromMemory(OrganicUIBinaryData::checkbox_off_png, OrganicUIBinaryData::checkbox_off_pngSize));
			setRepaintsOnMouseActivity(true);
		}
	}
}


void BoolToggleUI::valueChanged(const var&)
{
	shouldRepaint = true;
}

BoolButtonToggleUI::BoolButtonToggleUI(BoolParameter* parameter) :
	BoolToggleUI(parameter)
{
}

BoolButtonToggleUI::~BoolButtonToggleUI()
{
}

void BoolButtonToggleUI::paint(Graphics& g)
{
	if (parameter.wasObjectDeleted()) return;

	buttonRect = getLocalBounds().toFloat();
	if (!showLabel) buttonRect.setWidth(jmin<float>(buttonRect.getWidth(), buttonRect.getHeight() * 3));

	bool isOn = parameter->boolValue();

	Point<float> center = buttonRect.getCentre();

	Colour bgColor = useCustomBGColor ? customBGColor : NORMAL_COLOR;
	Colour c = bgColor.darker();
	Colour hc = useCustomFGColor ? customFGColor : HIGHLIGHT_COLOR;

	if (isInteractable())
	{
		if (isOn) c = hc;
		else c = isMouseOverOrDragging(true) ? (isMouseButtonDown() ? hc : bgColor.brighter()) : bgColor;
	}
	else
	{
		if (isOn) c = FEEDBACK_COLOR;
	}

	g.setGradientFill(ColourGradient(c, center.x, center.y, c.darker(.5f), 2.f, 2.f, true));
	g.fillRoundedRectangle(buttonRect.toFloat(), 4.f);
	g.setColour(c.darker());
	g.drawRoundedRectangle(buttonRect.toFloat(), 4.f, 2.f);

	if (showLabel)
	{
		juce::Rectangle<int> tr = getLocalBounds().reduced(2);
		g.setFont(jlimit(10, 40, jmin(tr.getHeight() - 4, tr.getWidth()) - 16));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, tr, Justification::centred, 3);
	}
}

bool BoolButtonToggleUI::hitTest(int x, int y)
{
	bool allowClicks, allowChildClicks;
	getInterceptsMouseClicks(allowClicks, allowChildClicks);
	if (!allowClicks) return false;
	return buttonRect.contains(x, y);;
}
