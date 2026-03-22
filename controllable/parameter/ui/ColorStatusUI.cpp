/*
  ==============================================================================

	ColorStatusUI.cpp
	Created: 11 Apr 2017 10:42:03am
	Author:  Ben

  ==============================================================================
*/


ColorStatusUI::ColorStatusUI(Array<Parameter*> parameters, bool isCircle) :
	ParameterUI(parameters, ORGANICUI_DEFAULT_TIMER),
	isCircle(isCircle),
	momentaryMode(false)
{
	setSize(32, 32);
}

ColorStatusUI::~ColorStatusUI()
{
}

void ColorStatusUI::paint(Graphics& g)
{
	int size = jmin(getWidth(), getHeight());
	Colour c = getCurrentColor();

	juce::Rectangle<float> r = (isCircle ? getLocalBounds().withSizeKeepingCentre(size, size) :getLocalBounds()).reduced(2).toFloat();

	g.setColour(c);
	if (isCircle) g.fillEllipse(r);
	else g.fillRoundedRectangle(r, 4);

	g.setColour(c.brighter(.3f));
	if (isCircle) g.drawEllipse(r, 2);
	else g.drawRoundedRectangle(r, 4, 2);

	if (showLabel)
	{
		if (customTextSize > 0) g.setFont(customTextSize);
		g.setColour(useCustomTextColor ? customTextColor : (isInteractable() ? TEXT_COLOR : FEEDBACK_COLOR));
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, getLocalBounds().reduced(2), Justification::centred, 3);
	}
}

Colour ColorStatusUI::getCurrentColor() const
{
	switch (parameter->type)
	{
	case Controllable::FLOAT:
	case Controllable::INT:
	case Controllable::BOOL:
	{
		if (parameter->colorStatusMap.contains(parameter->value)) return parameter->colorStatusMap[parameter->value];
		float minVal = INT32_MIN;
		HashMap<var, Colour>::Iterator it(parameter->colorStatusMap);
		Colour c = Colours::black.withAlpha(.4f);
		while (it.next())
		{
			float v = it.getKey();
			if (v <= parameter->floatValue() && v > minVal)
			{
				c = it.getValue();
				minVal = v;
			}
		}
		return c;
	}
	break;

	default:
		if (parameter->colorStatusMap.contains(parameter->value)) return parameter->colorStatusMap[parameter->value];
		break;
	}

	return Colours::black.withAlpha(.4f);
}

void ColorStatusUI::mouseDownInternal(const MouseEvent& e)
{
	if (!isInteractable()) return;
	if (parameter->type != Parameter::BOOL) return;

	if (e.mods.isLeftButtonDown())
	{
		if (e.mods.isAltDown() || momentaryMode) parameter->setValue(!parameter->boolValue());
		else parameter->setUndoableValue(parameter->boolValue(), !parameter->boolValue()); //only undoable when from left button, real toggle behaviour
	}
}

void ColorStatusUI::mouseUpInternal(const MouseEvent& e)
{
	if (!isInteractable()) return;
	if (parameter->type != Parameter::BOOL) return;

	if (e.mods.isLeftButtonDown())
	{
		if (e.mods.isAltDown() || momentaryMode) parameter->setValue(!parameter->boolValue());
	}
}

void ColorStatusUI::valueChanged(const var&)
{
	shouldRepaint = true;
}


//Options

ColorStatusUI::ColorOptionManager::ColorOptionManager(Parameter* parameter) :
	parameter(parameter)
{
	if (parameter->type != Controllable::BOOL)
	{
		HashMap<var, Colour>::Iterator it(parameter->colorStatusMap);
		while (it.next()) addOptionUI(it.getKey());
	}

	if (parameter->type == Controllable::FLOAT || parameter->type == Controllable::INT)
	{
		for (int i = 0; i < 5; i++) addOptionUI("");
	}
	else if (parameter->type == Controllable::BOOL)
	{
		addOptionUI(0);
		addOptionUI(1);
	}
	else if (parameter->type == Controllable::ENUM && optionsUI.size() == 0)
	{
		StringArray keys = ((EnumParameter*)parameter)->getAllKeys();
		for (auto& k : keys) addOptionUI(k);
	}

	updateColorOptions();

	viewport.setScrollBarsShown(true, false);
	addAndMakeVisible(&viewport);
	viewport.setViewedComponent(&container, false);

	setSize(200, 140);
}

ColorStatusUI::ColorOptionManager::~ColorOptionManager()
{
}

void ColorStatusUI::ColorOptionManager::addOptionUI(const var& key)
{
	ColorOptionUI* ui = new ColorOptionUI(parameter, key);
	optionsUI.add(ui);
	ui->keyLabel.addListener(this);
	ui->cp.addParameterListener(this);
	container.addAndMakeVisible(ui);
}


void ColorStatusUI::ColorOptionManager::paint(Graphics& g)
{
	juce::Rectangle<int> hr = getLocalBounds().removeFromTop(20);
	g.setColour(TEXT_COLOR);
	g.drawText("Value", hr.removeFromRight(getWidth() / 2).reduced(2).toFloat(), Justification::centred, false);
	g.drawText("Key", hr.reduced(2).toFloat(), Justification::centred, false);
}

void ColorStatusUI::ColorOptionManager::resized()
{
	juce::Rectangle<int> r = getLocalBounds().withHeight(20);
	for (int i = 0; i < optionsUI.size(); i++) optionsUI[i]->setBounds(r.translated(0, i * r.getHeight()));

	int th = optionsUI.size() * r.getHeight();
	container.setSize(getWidth() - 10, th);
	viewport.setBounds(getLocalBounds().withTrimmedTop(20));
}

void ColorStatusUI::ColorOptionManager::labelTextChanged(Label* l)
{
	updateColorOptions();
}

void ColorStatusUI::ColorOptionManager::parameterValueChanged(Parameter* p)
{
	updateColorOptions();
}

void ColorStatusUI::ColorOptionManager::updateColorOptions()
{
	parameter->colorStatusMap.clear();

	for (auto& o : optionsUI)
	{
		String kt = o->keyLabel.getText();
		if (kt.isEmpty()) continue;

		var k = parameter->type == Parameter::ENUM ? var(kt) : parameter->type == Parameter::BOOL ? var(kt.getIntValue()) : var(kt.getFloatValue());
		Colour v = o->cp.getColor();

		parameter->colorStatusMap.set(k, v);
	}
}

void ColorStatusUI::ColorOptionManager::show(Parameter* p, Component* c)
{
	std::unique_ptr<Component> editComponent(new ColorOptionManager(p));
	CallOutBox* box = &CallOutBox::launchAsynchronously(std::move(editComponent), c->localAreaToGlobal(c->getLocalBounds()), nullptr);
	box->setArrowSize(8);
}

ColorStatusUI::ColorOptionManager::ColorOptionUI::ColorOptionUI(Parameter* p, const var& key) :
	parameter(p),
	keyLabel("label", key.toString()),
	cp("Color", "Color for this key")
{
	if (p->colorStatusMap.contains(key)) cp.setColor(p->colorStatusMap[key]);

	keyLabel.setEditable(true);
	keyLabel.setBorderSize(BorderSize<int>(1));
	keyLabel.setColour(Label::backgroundColourId, BG_COLOR);
	keyLabel.setColour(Label::textColourId, TEXT_COLOR.darker(.2f));
	keyLabel.setColour(Label::textWhenEditingColourId, TEXT_COLOR);

	cpui.reset(cp.createColorParamUI());
	addAndMakeVisible(cpui.get());
	addAndMakeVisible(keyLabel);
}

void ColorStatusUI::ColorOptionManager::ColorOptionUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds().reduced(2);
	keyLabel.setBounds(r.removeFromLeft(getWidth() / 2).reduced(2));
	cpui->setBounds(r.reduced(2));
}

void ColorStatusUI::ColorOptionManager::ColorOptionUI::changeListenerCallback(ChangeBroadcaster* source)
{
	ColourSelector* s = dynamic_cast<ColourSelector*>(source);
	if (s == nullptr) return;
	cp.setColor(s->getCurrentColour());
}
