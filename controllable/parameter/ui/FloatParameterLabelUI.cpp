#include "FloatParameterLabelUI.h"
/*
  ==============================================================================

    FloatParameterLabelUI.cpp
    Created: 10 Dec 2016 10:51:19am
    Author:  Ben

  ==============================================================================
*/

FloatParameterLabelUI::FloatParameterLabelUI(Parameter * p) :
	ParameterUI(p),
	valueLabel(p->niceName + "_ValueLabel"),
	maxFontHeight(GlobalSettings::getInstance()->fontSize->floatValue()),
	autoSize(false)
{
	updateUIParams();

	addAndMakeVisible(&valueLabel);
	valueLabel.addListener(this);

	showEditWindowOnDoubleClick = false;

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue()+4);//default size

	valueChanged(parameter->getValue());	
	feedbackStateChanged();

	ParameterUI::setNextFocusOrder(&valueLabel);

	addMouseListener(this, true);
    
#if JUCE_MAC
    startTimerHz(10);
#else
    startTimerHz(20);
#endif

	timerCallback();
    
}

void FloatParameterLabelUI::setAutoSize(bool value)
{
	autoSize = value;
	valueChanged(parameter->getValue());
}

void FloatParameterLabelUI::setPrefix(const String & _prefix)
{
	if (prefix == _prefix) return;
	prefix = _prefix;
	valueChanged(parameter->stringValue());
}

void FloatParameterLabelUI::setSuffix(const String & _suffix)
{
	if (suffix == _suffix) return;
	suffix = _suffix;
	valueChanged(parameter->stringValue());
}



void FloatParameterLabelUI::feedbackStateChanged()
{
	updateUIParams();
}

void FloatParameterLabelUI::updateTooltip()
{
	ParameterUI::updateTooltip();
	valueLabel.setTooltip(tooltip);
}

/*
void FloatParameterLabelUI::paint(Graphics & g)
{
g.fillAll(Colours::purple.withAlpha(.2f));
}
*/

void FloatParameterLabelUI::resized()
{
	 juce::Rectangle<int> r = getLocalBounds();
	valueLabel.setFont(valueLabel.getFont().withHeight(jmin<float>((float)r.getHeight(), maxFontHeight)));
	valueLabel.setBounds(r);
	
}


void FloatParameterLabelUI::mouseDownInternal(const MouseEvent & e)
{
	valueAtMouseDown = parameter->floatValue();
	valueOffsetSinceMouseDown = 0;
	lastMouseX = e.getMouseDownX();
}


void FloatParameterLabelUI::mouseDrag(const MouseEvent & e)
{
	if (!isInteractable()) return;
	if (valueLabel.isBeingEdited()) return;

	if (valueLabel.getMouseCursor() != MouseCursor::LeftRightResizeCursor)
	{
		valueLabel.setMouseCursor(MouseCursor::LeftRightResizeCursor);
		valueLabel.updateMouseCursor();
	}

	float sensitivity = e.mods.isShiftDown() ? 10 : (e.mods.isAltDown() ? .1f : 1);
	
	valueOffsetSinceMouseDown += (e.getPosition().x - lastMouseX) * sensitivity / pixelsPerUnit;
	lastMouseX = e.getPosition().x;

	parameter->setValue(valueAtMouseDown + valueOffsetSinceMouseDown);
}

void FloatParameterLabelUI::mouseUpInternal(const MouseEvent & e)
{
	if (!isInteractable()) return;
	if (valueLabel.isBeingEdited()) return;  
	
	valueLabel.setMouseCursor(MouseCursor::NormalCursor);
	valueLabel.updateMouseCursor();

	if (valueAtMouseDown != parameter->floatValue()) parameter->setUndoableValue(valueAtMouseDown, parameter->floatValue());
}

void FloatParameterLabelUI::updateUIParams()
{
	valueLabel.setEditable(false, isInteractable(), false, true);
	valueLabel.setEnabled(isInteractable());
	
	valueLabel.setJustificationType(Justification::centred);
	valueLabel.setColour(Label::ColourIds::textColourId, useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : BLUE_COLOR.brighter(.2f)));

	valueLabel.setColour(valueLabel.backgroundColourId, useCustomBGColor?customBGColor:BG_COLOR.darker(.3f));
	valueLabel.setColour(valueLabel.backgroundWhenEditingColourId, Colours::black);
	valueLabel.setColour(CaretComponent::caretColourId, Colours::orange);
	valueLabel.setColour(valueLabel.textWhenEditingColourId, Colours::orange);
	valueLabel.setTooltip(tooltip);
	
	setOpaqueBackground(opaqueBackground); //force refresh color
}

void FloatParameterLabelUI::valueChanged(const var & v)
{
    valueString = v.isDouble()?String(parameter->floatValue(),3):v.toString();
	shouldUpdateLabel = true;
}

void FloatParameterLabelUI::labelTextChanged(Label *)
{
	//String  originalString = valueLabel.getText().substring(prefix.length(), valueLabel.getText().length() - suffix.length());
	parameter->setValue(valueLabel.getText().replace(",", ".").getFloatValue());
}


void FloatParameterLabelUI::timerCallback()
{
    if (!shouldUpdateLabel) return;
    shouldUpdateLabel = false;
    
	int newStyle = parameter->isOverriden ? Font::bold : Font::plain;
	if (valueLabel.getFont().getStyleFlags() != newStyle) valueLabel.setFont(valueLabel.getFont().withStyle(newStyle));


	valueLabel.setText(/*(showLabel ? parameter->niceName + " : " : "") + */prefix + valueString + suffix, NotificationType::dontSendNotification);
    
    if (autoSize)
    {
        int valueLabelWidth = valueLabel.getFont().getStringWidth(valueLabel.getText());
        int tw = valueLabelWidth;
        setSize(tw + 10, (int)valueLabel.getFont().getHeight());
    }
    
}



//TIME LABEL


TimeLabel::TimeLabel(Parameter * p) :
	FloatParameterLabelUI(p)
{
	valueChanged(parameter->getValue());
}

TimeLabel::~TimeLabel()
{
}

void TimeLabel::valueChanged(const var & v)
{
	String timeString = valueToTimeString(v);
	FloatParameterLabelUI::valueChanged(timeString);

}

void TimeLabel::labelTextChanged(Label *)
{
	parameter->setValue(timeStringToValue(valueLabel.getText()));
	valueLabel.setText(valueToTimeString(parameter->value), dontSendNotification);
}

#pragma warning (push)
#pragma warning(disable:4244)
String TimeLabel::valueToTimeString(float timeVal) const
{
	int hours = floor(timeVal / 3600);
	int minutes = floor(fmodf(timeVal, 3600) / 60);
	float seconds = fmodf(timeVal, 60);
	return String::formatted("%02i:%02i:%06.3f", hours, minutes, seconds);
}

float TimeLabel::timeStringToValue(String str) const
{
	StringArray sa;
	str = str.retainCharacters("0123456789.:;,");
	if (str.endsWithChar(':')) str += "0";
	sa.addTokens(str.replace(",","."), ":", "");

	float value = 0;

	value += sa.strings[sa.strings.size() - 1].getFloatValue();

	if (sa.strings.size() >= 2)
	{
		int numMinutes = sa.strings[sa.strings.size() - 2].getIntValue() * 60;
		value += numMinutes;
		if (sa.strings.size() >= 3)
		{
			int numHours = sa.strings[sa.strings.size() - 3].getFloatValue() * 3600;
			value += numHours;
		}
	}

	return value;
}


