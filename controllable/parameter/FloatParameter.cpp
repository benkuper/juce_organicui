/*
  ==============================================================================

    FloatParameter.cpp
    Created: 8 Mar 2016 1:22:10pm
    Author:  bkupe

  ==============================================================================
*/

FloatParameter::FloatParameter(const String & niceName, const String &description, const float & initialValue, const float & minValue, const float & maxValue, bool enabled) :
	Parameter(Type::FLOAT, niceName, description, (float)initialValue, (float)minValue, (float)maxValue, enabled),
	unitSteps(0),
    defaultUI(NONE),
    customUI(NONE)
{
	canHaveRange = true;
	canBeAutomated = true;
	argumentsDescription = "float";
}

var FloatParameter::getLerpValueTo(var targetValue, float weight)
{
	return jmap(weight,floatValue(), (float)targetValue);
}

void FloatParameter::setWeightedValue(Array<var> values, Array<float> weights)
{
	jassert(values.size() == weights.size());

	float tValue = 0;

	for (int i = 0; i < values.size(); ++i)
	{
		tValue += (float)values[i] * weights[i];
	}

	setValue(tValue);
}

FloatSliderUI * FloatParameter::createSlider(FloatParameter * target)
{
	if (target == nullptr) target = this;
    return new FloatSliderUI(target);
}

FloatStepperUI * FloatParameter::createStepper(FloatParameter * target)
{
	if (target == nullptr) target = this;
	return new FloatStepperUI(target);
}

FloatParameterLabelUI * FloatParameter::createLabelParameter(FloatParameter * target)
{
	if (target == nullptr) target = this;
	return new FloatParameterLabelUI(target);
}

TimeLabel * FloatParameter::createTimeLabelParameter(FloatParameter * target)
{
	if (target == nullptr) target = this;
	return new TimeLabel(target);
}

ControllableUI * FloatParameter::createDefaultUI() {
	UIType t = customUI != NONE ? customUI : defaultUI;

	if (t == NONE) t = hasRange()?SLIDER:LABEL;

	switch (t)
	{
    case NONE:
        break;
            
	case SLIDER:
		return createSlider(this);
		break;
	case STEPPER:
		return createStepper(this);
		break;
	case LABEL:
		return createLabelParameter(this);
		break;
	case TIME:
		return createTimeLabelParameter(this);
		break;
	}
	
	jassert(false);
	return nullptr;
}

bool FloatParameter::checkValueIsTheSame(var oldValue, var newValue)
{
	return jlimit<float>(minimumValue, maximumValue, newValue) == (float)oldValue;
}


void FloatParameter::setValueInternal(var& val)
{
	if (unitSteps > 0) value = round((float)val * unitSteps) / unitSteps;
	else Parameter::setValueInternal(val);
}

bool FloatParameter::hasRange()
{
	return (float)minimumValue != INT32_MIN && (float)maximumValue != INT32_MAX;
}

void FloatParameter::setControlAutomation()
{
	automation.reset(new ParameterNumberAutomation(this, !isLoadingData));
}

void FloatParameter::setAttribute(String attribute, var val)
{
	Parameter::setAttribute(attribute, val);
	if (attribute == "ui")
	{
		if (val == "time") defaultUI = TIME;
		else if (val == "slider") defaultUI = SLIDER;
		else if (val == "stepper") defaultUI = STEPPER;
		else if (val == "label") defaultUI = LABEL;
	}
	else if (attribute == "unitSteps")
	{
		unitSteps = (float)val;
	}
}

var FloatParameter::getJSONDataInternal()
{
	var data = Parameter::getJSONDataInternal();
	if (customUI != NONE) data.getDynamicObject()->setProperty("customUI", customUI);
	return data;
}

void FloatParameter::loadJSONDataInternal(var data)
{
	Parameter::loadJSONDataInternal(data);
	if (data.getDynamicObject()->hasProperty("defaultUI")) defaultUI = (UIType)(int)data.getProperty("defaultUI", SLIDER);
	customUI = (UIType)(int)data.getProperty("customUI", NONE);
}

var FloatParameter::getCroppedValue(var originalValue)
{
	return jlimit<float>(minimumValue, maximumValue, originalValue);
}

