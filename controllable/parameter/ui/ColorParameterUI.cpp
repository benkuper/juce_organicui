/*
  ==============================================================================

	ColorParameterUI.cpp
	Created: 11 Apr 2017 10:42:03am
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

ColorParameterUI::ColorParameterUI(Array<ColorParameter *> parameters) :
	ParameterUI(Inspectable::getArrayAs<ColorParameter, Parameter>(parameters), ORGANICUI_DEFAULT_TIMER),
    colorParams(parameters),
	colorParam(parameters[0]),
    dispatchOnDoubleClick(true),
	colorEditor(nullptr),
    dispatchOnSingleClick(false)
{
	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size
	showLabel = false;
}

ColorParameterUI::~ColorParameterUI()
{
	if(colorEditor != nullptr) colorEditor->removeComponentListener(this);

}

void ColorParameterUI::paint(Graphics & g)
{
	if (shouldBailOut()) return;


	juce::Rectangle<int> r = getLocalBounds();
	if (r.getWidth() < 2 || getHeight() < 2) return;

	Colour c = colorParam->getColor();
	int size = jmin(getWidth(), getHeight()) / 2;
	if (!c.isOpaque()) g.fillCheckerBoard(r.reduced(1).toFloat(), size, size, Colours::white, Colours::white.darker(.2f));
	
	g.setColour(c);
	g.fillRoundedRectangle(r.toFloat(), 2);

	if (isInteractable())
	{
		g.setColour(c.brighter(.5f).withAlpha(1.0f));
		g.drawRoundedRectangle(r.reduced(1).toFloat(), 2, 2);
	}

	if (showLabel)
	{
		juce::Rectangle<int> tr = r.reduced(2);
		g.setFont(jlimit(12, 40, jmin(tr.getHeight(), tr.getWidth()) - 16));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : colorParam->niceName, tr, Justification::centred, 1);
	}
	
}

void ColorParameterUI::resized()
{

}

void ColorParameterUI::mouseDownInternal(const MouseEvent & e)
{
	if (dispatchOnSingleClick) showEditWindow();
}

void ColorParameterUI::showEditWindowInternal()
{
	if (!isInteractable()) return;
	if (shouldBailOut()) return;

	std::unique_ptr<ColourSelector> selector(new ColourSelector(ColourSelector::showAlphaChannel | ColourSelector::showSliders | ColourSelector::showHexColorValue | ColourSelector::showColourspace));
	selector->addChangeListener(this);
	selector->setName("Color for " + parameter->niceName);
	selector->setCurrentColour(colorParam->getColor());
	selector->setColour(ColourSelector::backgroundColourId, Colours::transparentBlack);
	selector->setColour(ColourSelector::labelTextColourId, TEXT_COLOR);
	selector->setSize(300, 400);
	selector->addChangeListener(this);
	valueOnEditorOpen = colorParam->getValue();
	colorEditor = &CallOutBox::launchAsynchronously(std::move(selector), getScreenBounds(), nullptr);
	colorEditor->addComponentListener(this);
}

void ColorParameterUI::showEditRangeWindowInternal()
{
	if (!parameter->isCustomizableByUser) return;

	AlertWindow* nameWindow = new AlertWindow("Change color bounds", "Set new bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[4]{ "Red", "Green", "Blue", "Alpha" };

	for (int i = 0; i < 4; ++i)
	{
		nameWindow->addTextEditor("minVal" + String(i), String((float)colorParam->minimumValue[i]), "Minimum " + coordNames[i]);
		nameWindow->addTextEditor("maxVal" + String(i), String((float)colorParam->maximumValue[i]), "Maximum " + coordNames[i]);
	}

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	nameWindow->enterModalState(true, ModalCallbackFunction::create([this, nameWindow](int result)
		{
			if (result != 1) return;

			float newMins[4];
			float newMaxs[4];
			for (int i = 0; i < 4; ++i)
			{
				newMins[i] = nameWindow->getTextEditorContents("minVal" + String(i)).getFloatValue();
				newMaxs[i] = nameWindow->getTextEditorContents("maxVal" + String(i)).getFloatValue();
			}
			colorParam->setBounds(newMins[0], newMins[1], newMins[2], newMins[3],
								  jmax(newMins[0], newMaxs[0]),
								  jmax(newMins[1], newMaxs[1]),
								  jmax(newMins[2], newMaxs[2]),
								  jmax(newMins[3], newMaxs[3]));
		}
	), true);
}

void ColorParameterUI::componentBeingDeleted(Component& c)
{
	if (&c == colorEditor)
	{
		colorParam->setUndoableValue(valueOnEditorOpen, colorParam->value);
		colorEditor = nullptr;
	}
}

void ColorParameterUI::valueChanged(const var &)
{
	shouldRepaint = true;
}

void ColorParameterUI::changeListenerCallback(ChangeBroadcaster * source)
{
	ColourSelector * s = dynamic_cast<ColourSelector *>(source);
	if (s == nullptr || shouldBailOut()) return;
	colorParam->setColor(s->getCurrentColour());

}

