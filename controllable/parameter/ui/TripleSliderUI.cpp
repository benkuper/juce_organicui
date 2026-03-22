/*
  ==============================================================================

	TripleSliderUI.cpp
	Created: 25 Oct 2016 11:46:46am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

TripleSliderUI::TripleSliderUI(Array<Point3DParameter*> parameters) :
	ParameterUI(Inspectable::getArrayAs<Point3DParameter, Parameter>(parameters)),
	p3ds(parameters),
	p3d(parameters[0]),
	xParam("X", "xParam", p3d->x, p3d->minimumValue[0], p3d->maximumValue[0]),
	yParam("Y", "yParam", p3d->y, p3d->minimumValue[1], p3d->maximumValue[1]),
	zParam("Z", "zParam", p3d->z, p3d->minimumValue[2], p3d->maximumValue[2]),
	isUpdatingFromParam(false)
{

	showEditWindowOnDoubleClick = false;

	xParam.canHaveRange = p3d->canHaveRange;
	yParam.canHaveRange = p3d->canHaveRange;
	zParam.canHaveRange = p3d->canHaveRange;

	xParam.isCustomizableByUser = p3d->isCustomizableByUser;
	yParam.isCustomizableByUser = p3d->isCustomizableByUser;
	zParam.isCustomizableByUser = p3d->isCustomizableByUser;

	xParam.defaultValue = 0;
	yParam.defaultValue = 0;
	zParam.defaultValue = 0;

	xParam.defaultUI = p3d->defaultUI;
	yParam.defaultUI = p3d->defaultUI;
	zParam.defaultUI = p3d->defaultUI;

	xParam.stringDecimals = p3d->stringDecimals;
	yParam.stringDecimals = p3d->stringDecimals;
	zParam.stringDecimals = p3d->stringDecimals;

	xParam.addAsyncParameterListener(this);
	yParam.addAsyncParameterListener(this);
	zParam.addAsyncParameterListener(this);

	xSlider.reset((ParameterUI*)xParam.createDefaultUI());
	ySlider.reset((ParameterUI*)yParam.createDefaultUI());
	zSlider.reset((ParameterUI*)zParam.createDefaultUI());

	xSlider->showLabel = false;
	ySlider->showLabel = false;
	zSlider->showLabel = false;

	xSlider->showMenuOnRightClick = false;
	ySlider->showMenuOnRightClick = false;
	zSlider->showMenuOnRightClick = false;

	xSlider->setUndoableValueOnMouseUp = false;
	ySlider->setUndoableValueOnMouseUp = false;
	zSlider->setUndoableValueOnMouseUp = false;

	xSlider->addMouseListener(this, true);
	ySlider->addMouseListener(this, true);
	zSlider->addMouseListener(this, true);

	addAndMakeVisible(xSlider.get());
	addAndMakeVisible(ySlider.get());
	addAndMakeVisible(zSlider.get());

	setSize(200, GlobalSettings::getInstance()->fontSize->floatValue() + 4);//default size

	updateUIParams(); //force update
}

TripleSliderUI::~TripleSliderUI()
{
	xParam.removeAsyncParameterListener(this);
	yParam.removeAsyncParameterListener(this);
	zParam.removeAsyncParameterListener(this);
}

void TripleSliderUI::mouseDownInternal(const MouseEvent&)
{
	mouseDownValue = parameter->getValue();
}

void TripleSliderUI::mouseUpInternal(const MouseEvent&)
{
	if (setUndoableValueOnMouseUp)
	{
		if ((float)mouseDownValue[0] != xParam.floatValue()
			|| (float)mouseDownValue[1] != yParam.floatValue()
			|| (float)mouseDownValue[2] != zParam.floatValue())
		{

			p3d->setUndoableVector((float)mouseDownValue[0], (float)mouseDownValue[1], (float)mouseDownValue[2], xParam.floatValue(), yParam.floatValue(), zParam.floatValue());
		}
	}
}

void TripleSliderUI::paint(Graphics& g)
{
	ParameterUI::paint(g);

	if (showLabel)
	{
		juce::Rectangle<int> r = getLocalBounds();
		g.setFont(jlimit(12, 40, jmin(r.getHeight(), r.getWidth()) - 16));
		r = r.removeFromLeft(jmin(g.getCurrentFont().getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 60));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, r, Justification::centred, 1);
	}
}

void TripleSliderUI::resized()
{
	juce::Rectangle<int> r = getLocalBounds();

	if (showLabel)
	{
		Font font(jlimit(12, 40, jmin(r.getHeight(), r.getWidth()) - 16));
		r.removeFromLeft(jmin(font.getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, r.getWidth() - 60));
		r.removeFromLeft(2);
	}

	xSlider->setBounds(r.removeFromLeft(r.getWidth() / 3 - 10));
	r.removeFromLeft(5);
	ySlider->setBounds(r.removeFromLeft(r.getWidth() / 2 - 5));
	r.removeFromLeft(5);
	zSlider->setBounds(r);
}

void TripleSliderUI::showEditWindowInternal()
{
	AlertWindow* nameWindow = new AlertWindow("Change point 3D values", "Set new values for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[3]{ "X","Y","Z" };

	for (int i = 0; i < 3; ++i) nameWindow->addTextEditor("val" + String(i), String((float)p3d->value[i]), "Value " + coordNames[i]);

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	Point3DParameter* param = p3d;

	nameWindow->enterModalState(true, ModalCallbackFunction::create([param, nameWindow](int result)
		{

			if (result)
			{
				float newVals[3];
				for (int i = 0; i < 3; ++i) newVals[i] = nameWindow->getTextEditorContents("val" + String(i)).getFloatValue();
				param->setUndoableVector(param->x, param->y, param->z, newVals[0], newVals[1], newVals[2]);
			}
		}
	), true);
}

void TripleSliderUI::showEditRangeWindowInternal()
{
	if (!parameter->isCustomizableByUser) return;

	AlertWindow* nameWindow = new AlertWindow("Change point 3D bounds", "Set new bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[3]{ "X","Y","Z" };

	for (int i = 0; i < 3; ++i)
	{
		nameWindow->addTextEditor("minVal" + String(i), String((float)p3d->minimumValue[i]), "Minimum " + coordNames[i]);
		nameWindow->addTextEditor("maxVal" + String(i), String((float)p3d->maximumValue[i]), "Maximum " + coordNames[i]);
	}

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	nameWindow->enterModalState(true, ModalCallbackFunction::create([this, nameWindow](int result)
		{

			if (result)
			{
				float newMins[3];
				float newMaxs[3];
				for (int i = 0; i < 3; ++i)
				{
					newMins[i] = nameWindow->getTextEditorContents("minVal" + String(i)).getFloatValue();
					newMaxs[i] = nameWindow->getTextEditorContents("maxVal" + String(i)).getFloatValue();
				}
				p3d->setBounds(newMins[0], newMins[1], newMins[2], jmax(newMins[0], newMaxs[0]), jmax(newMins[1], newMaxs[1]), jmax(newMins[2], newMaxs[2]));
			}
		}
	), true);
}


void TripleSliderUI::rangeChanged(Parameter* p)
{
	if (p != parameter) return;
	isUpdatingFromParam = true;
	xParam.setRange(parameter->minimumValue[0], parameter->maximumValue[0]);
	yParam.setRange(parameter->minimumValue[1], parameter->maximumValue[1]);
	zParam.setRange(parameter->minimumValue[2], parameter->maximumValue[2]);
	isUpdatingFromParam = false;
}

void TripleSliderUI::updateUIParamsInternal()
{
	xParam.setControllableFeedbackOnly(!isInteractable());
	yParam.setControllableFeedbackOnly(!isInteractable());
	zParam.setControllableFeedbackOnly(!isInteractable());

	Array<ParameterUI*> puis{ xSlider.get(), ySlider.get(), zSlider.get() };

	for (auto& u : puis)
	{
		u->useCustomBGColor = useCustomBGColor;
		u->customBGColor = customBGColor;
		u->useCustomFGColor = useCustomFGColor;
		u->customFGColor = customFGColor;
		u->useCustomTextColor = useCustomTextColor;
		u->customTextColor = customTextColor;
		u->showValue = showValue;
		u->showLabel = showLabel;
		u->updateUIParams();
	}
}

void TripleSliderUI::newMessage(const Parameter::ParameterEvent& e)
{
	ParameterUI::newMessage(e);

	if (e.parameter == parameter)
	{
		isUpdatingFromParam = true;
		xParam.setValue(((Point3DParameter*)e.parameter)->x);
		yParam.setValue(((Point3DParameter*)e.parameter)->y);
		zParam.setValue(((Point3DParameter*)e.parameter)->z);
		isUpdatingFromParam = false;
	}
	else if (isInteractable() && !isUpdatingFromParam)
	{
		if (e.parameter == &xParam || e.parameter == &yParam || e.parameter == &zParam)
		{
			if (xParam.floatValue() != p3d->x || yParam.floatValue() != p3d->y || zParam.floatValue() != p3d->z)
			{
				if (!isMouseButtonDown(true) && !UndoMaster::getInstance()->isPerformingUndoRedo()) p3d->setUndoableVector(p3d->x, p3d->y, p3d->z, xParam.floatValue(), yParam.floatValue(), zParam.floatValue());
				else p3d->setVector(xParam.floatValue(), yParam.floatValue(), zParam.floatValue());
			}
		}
	}
}
