/*
  ==============================================================================

	DoubleSliderUI.cpp
	Created: 25 Oct 2016 11:46:46am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

using namespace juce;

DoubleSliderUI::DoubleSliderUI(Array<Point2DParameter*> parameters) :
	ParameterUI(Inspectable::getArrayAs<Point2DParameter, Parameter>(parameters)),
	p2ds(parameters),
	p2d(parameters[0]),
	xParam("X", "xParam", p2d->x, p2d->minimumValue[0], p2d->maximumValue[0]),
	yParam("Y", "yParam", p2d->y, p2d->minimumValue[1], p2d->maximumValue[1]),
	canShowExtendedEditor(p2d->hasRange()),
	isUpdatingFromParam(false)

{

	showEditWindowOnDoubleClick = false;

	xParam.canHaveRange = parameter->canHaveRange;
	yParam.canHaveRange = parameter->canHaveRange;
	xParam.isCustomizableByUser = parameter->isCustomizableByUser;
	yParam.isCustomizableByUser = parameter->isCustomizableByUser;
	xParam.defaultValue = 0;
	yParam.defaultValue = 0;
	xParam.defaultUI = p2d->defaultUI;
	yParam.defaultUI = p2d->defaultUI;


	xParam.addAsyncParameterListener(this);
	yParam.addAsyncParameterListener(this);
	xParam.stringDecimals = p2d->stringDecimals;
	yParam.stringDecimals = p2d->stringDecimals;

	xSlider.reset((ParameterUI*)xParam.createDefaultUI());
	ySlider.reset((ParameterUI*)yParam.createDefaultUI());

	xSlider->showLabel = false;
	ySlider->showLabel = false;
	xSlider->showMenuOnRightClick = false;
	ySlider->showMenuOnRightClick = false;

	xSlider->setUndoableValueOnMouseUp = false;
	ySlider->setUndoableValueOnMouseUp = false;

	xSlider->addMouseListener(this, true);
	ySlider->addMouseListener(this, true);

	addAndMakeVisible(xSlider.get());
	addAndMakeVisible(ySlider.get());

	float baseHeight = GlobalSettings::getInstance()->fontSize->floatValue() + 4;
	setSize(200, baseHeight);//default size

	updateUIParams(); //force update
}

DoubleSliderUI::~DoubleSliderUI()
{
	xParam.removeAsyncParameterListener(this);
	yParam.removeAsyncParameterListener(this);

}

void DoubleSliderUI::mouseDownInternal(const MouseEvent& e)
{
	if (canShowExtendedEditor && p2d->canShowExtendedEditor && canvasSwitchRect.contains(e.getMouseDownPosition().toFloat()))
	{
		setUndoableValueOnMouseUp = false;
		p2d->setShowExtendedEditor(!p2d->showExtendedEditor);
	}
	else
	{
		setUndoableValueOnMouseUp = true;
		mouseDownValue = parameter->getValue();
	}
}

void DoubleSliderUI::mouseUpInternal(const MouseEvent&)
{
	if (setUndoableValueOnMouseUp)
	{
		if ((float)mouseDownValue[0] != xParam.floatValue() || (float)mouseDownValue[1] != yParam.floatValue()) p2d->setUndoablePoint((float)mouseDownValue[0], (float)mouseDownValue[1], xParam.floatValue(), yParam.floatValue());
	}
}

void DoubleSliderUI::paint(Graphics& g)
{
	ParameterUI::paint(g);

	float baseHeight = GlobalSettings::getInstance()->fontSize->floatValue() + 4;
	juce::Rectangle<int> r = getLocalBounds();
	juce::Rectangle<int> sr = canvasUI == nullptr ? r : r.removeFromTop(baseHeight);

	if (showLabel)
	{
		g.setFont(jlimit(12, 40, jmin(sr.getHeight(), sr.getWidth()) - 16));
		juce::Rectangle<int> tr = sr.removeFromLeft(jmin(g.getCurrentFont().getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, sr.getWidth() - 60));
		g.setColour(useCustomTextColor ? customTextColor : TEXT_COLOR);
		g.drawFittedText(customLabel.isNotEmpty() ? customLabel : parameter->niceName, tr, Justification::centred, 1);
		sr.removeFromLeft(2);
	}

	if (canShowExtendedEditor && p2d->canShowExtendedEditor)
	{
		juce::Rectangle<float> rr = canvasSwitchRect.reduced(4);
		Path p;
		if (canvasUI == nullptr) p.addTriangle(rr.getTopLeft(), Point<float>(rr.getRight(), rr.getCentreY()), rr.getBottomLeft());
		else p.addTriangle(rr.getTopLeft(), rr.getTopRight(), Point<float>(rr.getCentreX(), rr.getBottom()));
		g.setColour(isMouseOverOrDragging() && rr.contains(getMouseXYRelative().toFloat()) ? NORMAL_COLOR.brighter() : NORMAL_COLOR);
		g.fillPath(p.createPathWithRoundedCorners(3));
	}
}

void DoubleSliderUI::resized()
{
	float baseHeight = GlobalSettings::getInstance()->fontSize->floatValue() + 4;
	juce::Rectangle<int> r = getLocalBounds();
	juce::Rectangle<int> sr = canvasUI == nullptr ? r : r.removeFromTop(baseHeight);
	if (showLabel)
	{
		Font font(jlimit(12, 40, jmin(sr.getHeight(), sr.getWidth()) - 16));
		sr.removeFromLeft(jmin(font.getStringWidth(customLabel.isNotEmpty() ? customLabel : parameter->niceName) + 10, sr.getWidth() - 60));
		sr.removeFromLeft(2);
	}

	if (canShowExtendedEditor && p2d->canShowExtendedEditor) canvasSwitchRect = sr.removeFromLeft(sr.getHeight()).toFloat();

	xSlider->setBounds(sr.removeFromLeft(sr.getWidth() / 2 - 5));
	ySlider->setBounds(sr.removeFromRight(sr.getWidth() - 10));

	if (canvasUI != nullptr)
	{
		r.removeFromTop(2);
		canvasUI->setBounds(r);
	}
}

void DoubleSliderUI::showEditWindowInternal()
{
	AlertWindow* nameWindow = new AlertWindow("Change point 2D values", "Set new values for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; ++i) nameWindow->addTextEditor("val" + String(i), String((float)p2d->value[i]), "Value " + coordNames[i]);

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	Point2DParameter* param = p2d;

	nameWindow->enterModalState(true, ModalCallbackFunction::create([param, nameWindow](int result)
		{
			if (result)
			{
				float newVals[2];
				for (int i = 0; i < 2; ++i) newVals[i] = nameWindow->getTextEditorContents("val" + String(i)).getFloatValue();
				param->setUndoablePoint(param->x, param->y, newVals[0], newVals[1]);
			}
		}),
		true
			);
}

void DoubleSliderUI::showEditRangeWindowInternal()
{
	if (!parameter->isCustomizableByUser) return;

	AlertWindow* nameWindow = new AlertWindow("Change point 2D bounds", "Set new bounds for this parameter", AlertWindow::AlertIconType::NoIcon, this);

	const String coordNames[2]{ "X","Y" };

	for (int i = 0; i < 2; ++i)
	{
		nameWindow->addTextEditor("minVal" + String(i), String((float)p2d->minimumValue[i]), "Minimum " + coordNames[i]);
		nameWindow->addTextEditor("maxVal" + String(i), String((float)p2d->maximumValue[i]), "Maximum " + coordNames[i]);
	}

	nameWindow->addButton("OK", 1, KeyPress(KeyPress::returnKey));
	nameWindow->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));

	nameWindow->enterModalState(true, ModalCallbackFunction::create([this, nameWindow](int result)
		{
			float newMins[2];
			float newMaxs[2];
			for (int i = 0; i < 2; ++i)
			{
				newMins[i] = nameWindow->getTextEditorContents("minVal" + String(i)).getFloatValue();
				newMaxs[i] = nameWindow->getTextEditorContents("maxVal" + String(i)).getFloatValue();
			}
			p2d->setBounds(newMins[0], newMins[1], jmax(newMins[0], newMaxs[0]), jmax(newMins[1], newMaxs[1]));

		}
	), true);
}

void DoubleSliderUI::updateUseExtendedEditor()
{
	float baseHeight = GlobalSettings::getInstance()->fontSize->floatValue() + 4;

	if (p2d->showExtendedEditor && canShowExtendedEditor)
	{
		if (canvasUI == nullptr)
		{
			canvasUI.reset(new P2DUI(p2d));
			addAndMakeVisible(canvasUI.get());
			setSize(getWidth(), baseHeight + 2 + getWidth());//default size
		}
	}
	else
	{
		if (canvasUI != nullptr)
		{
			removeChildComponent(canvasUI.get());
			canvasUI.reset();
			setSize(getWidth(), baseHeight);//default size
		}
	}
}


void DoubleSliderUI::rangeChanged(Parameter* p)
{
	if (p != parameter) return;
	isUpdatingFromParam = true;
	xParam.setRange(parameter->minimumValue[0], parameter->maximumValue[0]);
	yParam.setRange(parameter->minimumValue[1], parameter->maximumValue[1]);
	isUpdatingFromParam = false;
}

void DoubleSliderUI::updateUIParamsInternal()
{
	xParam.setControllableFeedbackOnly(!isInteractable());
	yParam.setControllableFeedbackOnly(!isInteractable());

	Array<ParameterUI*> puis{ xSlider.get(), ySlider.get() };

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

	updateUseExtendedEditor();
}

void DoubleSliderUI::newMessage(const Parameter::ParameterEvent& e)
{
	ParameterUI::newMessage(e);

	if (e.type == e.UI_PARAMS_CHANGED)
	{
		updateUseExtendedEditor();
	}

	if (e.parameter == parameter)
	{
		isUpdatingFromParam = true;
		xParam.setValue(((Point2DParameter*)e.parameter)->x);
		yParam.setValue(((Point2DParameter*)e.parameter)->y);
		isUpdatingFromParam = false;
	}
	else if (isInteractable() && !isUpdatingFromParam)
	{
		if (e.parameter == &xParam || e.parameter == &yParam)
		{
			if (xParam.floatValue() != p2d->x || yParam.floatValue() != p2d->y)
			{
				if (!isMouseButtonDown(true) && !UndoMaster::getInstance()->isPerformingUndoRedo()) p2d->setUndoablePoint(p2d->x, p2d->y, xParam.floatValue(), yParam.floatValue());
				else p2d->setPoint(xParam.floatValue(), yParam.floatValue());
			}
		}
	}
}
