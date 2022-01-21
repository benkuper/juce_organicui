/*
  ==============================================================================

	ColorParameterUI.cpp
	Created: 11 Apr 2017 10:42:03am
	Author:  Ben

  ==============================================================================
*/


ColorParameterUI::ColorParameterUI(Array<ColorParameter *> parameters) :
	ParameterUI(Inspectable::getArrayAs<ColorParameter, Parameter>(parameters)),
    colorParams(parameters),
	colorParam(parameters[0]),
    dispatchOnDoubleClick(true),
	dispatchOnSingleClick(false),
	colorEditor(nullptr)
{
	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size
	showLabel = false;
}

ColorParameterUI::~ColorParameterUI()
{
}

void ColorParameterUI::paint(Graphics & g)
{
	if (shouldBailOut()) return;


	Rectangle<int> r = getLocalBounds();
	if (r.getWidth() < 2 || getHeight() < 2) return;

	Colour c = colorParam->getColor();
	int size = jmin(getWidth(), getHeight()) / 2;
	if (!c.isOpaque()) g.fillCheckerBoard(r.reduced(1).toFloat(), size, size, Colours::white, Colours::white.darker(.2f));
	
	g.setColour(c);
	g.fillRoundedRectangle(r.toFloat(), 2);

	if (isInteractable())
	{
		g.setColour(c.brighter(.5f));
		g.drawRoundedRectangle(r.reduced(1).toFloat(), 2, 2);
	}

	if (showLabel)
	{
		Rectangle<int> tr = r.reduced(2);
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
	repaint();
}

void ColorParameterUI::changeListenerCallback(ChangeBroadcaster * source)
{
	ColourSelector * s = dynamic_cast<ColourSelector *>(source);
	if (s == nullptr) return;
	colorParam->setColor(s->getCurrentColour());

}

