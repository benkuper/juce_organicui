/*
  ==============================================================================

	fKeyUI.cpp
	Created: 21 Mar 2020 4:06:36pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"


Curve2DKeyUI::Curve2DKeyUI(Curve2DKey* key) :
	BaseItemMinimalUI(key),
	handle(key),
	easingUI(nullptr)
{
	bringToFrontOnSelect = false;

	drawEmptyDragIcon = true;
	autoHideWhenDragging = false;
	autoDrawContourWhenSelected = false;
	dragStartDistance = 5;
	addAndMakeVisible(&handle);

	updateEasingUI();
}

Curve2DKeyUI::~Curve2DKeyUI()
{
	if (!inspectable.wasObjectDeleted()) if (item->easing != nullptr) item->easing->removeInspectableListener(this);
}

void Curve2DKeyUI::resized()
{
	Point<int> hp = getUIPosForValuePos(item->viewUIPosition->getPoint());
	const int handleSize = 14;
	handle.setBounds(juce::Rectangle<int>(hp.x - handleSize / 2, hp.y - handleSize / 2, handleSize, handleSize));
	if (easingUI != nullptr) easingUI->setBounds(getLocalBounds());
}

void Curve2DKeyUI::paint(Graphics& g)
{
}

void Curve2DKeyUI::setShowEasingHandles(bool showFirst, bool showLast)
{
	if (inspectable.wasObjectDeleted()) return;
	if (easingUI != nullptr) easingUI->setShowEasingHandles(showFirst && item->nextKey != nullptr, showLast && item->nextKey != nullptr);
}

void Curve2DKeyUI::setFocus(float relPos, float range)
{
	if (range == 0)
	{
		handle.setAlpha(1);
		if (easingUI != nullptr) easingUI->setFocus(0, 0);
		return;
	}

	float d = jlimit<float>(0, 1, 1 - (fabsf(relPos) / (range / 2)));
	handle.setAlpha(d);
	if (easingUI != nullptr) easingUI->setFocus(relPos, range);
}

void Curve2DKeyUI::updateEasingUI()
{
	if (easingUI != nullptr)
	{
		removeChildComponent(this);
	}

	if (item->easing == nullptr)
	{
		easingUI.reset();
		return;
	}

	easingUI.reset(item->easing->createUI());

	if (easingUI != nullptr)
	{
		addAndMakeVisible(easingUI.get(), 0);
		easingUI->setBounds(getLocalBounds());
		easingUI->setValueBounds(valueBounds);
	}

}

void Curve2DKeyUI::mouseDown(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDown(e);

	if (e.eventComponent == easingUI.get())
	{
		if (e.mods.isRightButtonDown())
		{
			PopupMenu ep;
			juce::StringArray keys = item->easingType->getAllKeys();
			int kid = 1;
			for (auto& i : keys)
			{
				ep.addItem(kid, i, true, i == item->easingType->getValueKey());
				kid++;
			}

			ep.showMenuAsync(PopupMenu::Options(), [this, keys](int result)
				{
					if (result >= 1 && result <= keys.size())
					{
						this->item->easingType->setUndoableValue(item->easingType->value, keys[result - 1]);
						this->item->easing->selectThis(); //reselect after changing easing
					}
				}
			);

		}
		else if (e.mods.isCommandDown())
		{
			item->easingType->setNext(true, true);
			item->easing->selectThis(); //reselect after changing easing
		}
	}

}

void Curve2DKeyUI::mouseDoubleClick(const MouseEvent& e)
{
	if (e.eventComponent == this || e.eventComponent == &handle)
	{
		std::unique_ptr<Component> editComponent(new ParameterUI::ValueEditCalloutComponent(item->viewUIPosition));
		CallOutBox* box = &CallOutBox::launchAsynchronously(std::move(editComponent), localAreaToGlobal(getLocalBounds()), nullptr);
		box->setArrowSize(8);
	}

}

bool Curve2DKeyUI::canStartDrag(const MouseEvent& e)
{
	return e.eventComponent == this || e.eventComponent == &handle;
}

Point<int> Curve2DKeyUI::getDragOffset()
{
	return Point<int>(); //no offset, mouse position should be the position of the curve point
}

bool Curve2DKeyUI::hitTest(int x, int y)
{
	if (handle.getBounds().contains(x, y)) return true;
	if (easingUI != nullptr && easingUI->hitTest(x, y)) return true;
	return false;
}

void Curve2DKeyUI::setValueBounds(const juce::Rectangle<float> _valueBounds)
{
	valueBounds = _valueBounds;
	if (easingUI != nullptr) easingUI->setValueBounds(valueBounds);
	resized();
}


Point<int> Curve2DKeyUI::getUIPosForValuePos(const Point<float>& valuePos) const
{
	return getLocalBounds().getRelativePoint((valuePos.x - valueBounds.getX()) / valueBounds.getWidth(), (valuePos.y - valueBounds.getY()) / valueBounds.getHeight());
}


void Curve2DKeyUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == item->easingType) updateEasingUI();
	else if (CubicEasing2D* ce = dynamic_cast<CubicEasing2D*>(c->parentContainer.get()))
	{
		if (CubicEasing2DUI* eui = dynamic_cast<CubicEasing2DUI*>(easingUI.get()))
		{
			bool isFirst = c == ce->anchor1;
			keyUIListeners.call(&KeyUIListener::keyEasingHandleMoved, this, eui->syncHandles, isFirst);
		}
	}
}

Curve2DKeyHandle::Curve2DKeyHandle(Curve2DKey* key) :
	InspectableContentComponent(key),
	key(key)
{
	autoDrawContourWhenSelected = false;
	setRepaintsOnMouseActivity(true);

}

Curve2DKeyHandle::~Curve2DKeyHandle()
{
}

void Curve2DKeyHandle::paint(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	Colour bc = key->isFirst ? GREEN_COLOR : (key->nextKey == nullptr ? YELLOW_COLOR : NORMAL_COLOR);
	Colour c = key->isSelected ? HIGHLIGHT_COLOR : key->isPreselected ? PRESELECT_COLOR : bc;
	if (isMouseOverOrDragging()) c = c.brighter(.2f);

	juce::Rectangle<float> r = getLocalBounds().reduced(3).toFloat();
	g.setColour(c);
	g.fillEllipse(r);
	g.setColour(c.darker());
	g.drawEllipse(r, 1);
}

void Curve2DKeyHandle::inspectableSelectionChanged(Inspectable* i)
{
	repaint();
}
