/*
  ==============================================================================

	Curve2DUI.cpp
	Created: 21 Mar 2020 4:06:30pm
	Author:  bkupe

  ==============================================================================
*/


#include "JuceHeader.h"


Curve2DUI::Curve2DUI(Curve2D* manager) :
	BaseManagerViewUI(manager->niceName, manager),
	paintingMode(false)
{
	useCheckersAsUnits = true;
	minZoom = .1f;
	maxZoom = 10;

	setShowAddButton(false);

	updatePositionOnDragMove = true;
	animateItemOnAdd = false;
	manager->addAsyncContainerListener(this);
	if (manager->recorder != nullptr) manager->recorder->addAsyncCoalescedRecorderListener(this);

	setSize(100, 300);
	addExistingItems(false);
}

Curve2DUI::~Curve2DUI()
{
	if (!inspectable.wasObjectDeleted())
	{
		manager->removeAsyncContainerListener(this);
		if (manager->recorder != nullptr) manager->recorder->removeAsyncRecorderListener(this);
	}

	for (auto& ui : itemsUI)
	{
		if (ui != nullptr && !ui->inspectable.wasObjectDeleted())
		{
			ui->item->removeAsyncKeyListener(this);
			ui->removeKeyUIListener(this);
		}
	}
}

void Curve2DUI::paintOverChildren(Graphics& g)
{
	if (inspectable.wasObjectDeleted()) return;

	g.setColour(GREEN_COLOR);
	g.drawEllipse(juce::Rectangle<int>(0, 0, 8, 8).withCentre(getPosInView(manager->value->getPoint())).toFloat(), 2);

	if (paintingMode && paintingPoints.size() > 0)
	{
		g.setColour(YELLOW_COLOR);
		Path p;
		p.startNewSubPath(getPosInView(paintingPoints[0]).toFloat());
		for (auto& pp : paintingPoints)
		{
			Point<int> vpp = getPosInView(pp);
			g.fillEllipse(juce::Rectangle<int>(0, 0, 2, 2).withCentre(vpp).toFloat());
			p.lineTo(vpp.toFloat());
		}

		g.setColour(YELLOW_COLOR.withAlpha(.5f));
		g.strokePath(p, PathStrokeType(1));
	}

	//recorder
	if (manager->recorder != nullptr)
	{
		if (manager->recorder->isRecording->boolValue())
		{
			int numRKeys = manager->recorder->keys.size();
			if (numRKeys >= 2 && manager->recorder->keys[0].value.isArray())
			{
				Path p;
				Point<float> k0(manager->recorder->keys[0].value[0], manager->recorder->keys[0].value[1]);
				p.startNewSubPath(getPosInView(k0).toFloat());
				for (int i = 1; i < numRKeys; ++i)
				{
					if (!manager->recorder->keys[i].value.isArray()) continue;
					Point<float> ki(manager->recorder->keys[i].value[0], manager->recorder->keys[i].value[1]);
					p.lineTo(getPosInView(ki).toFloat());
				}
				g.setColour(Colours::orangered);
				g.strokePath(p, PathStrokeType(2));
			}
		}
	}
}

void Curve2DUI::updateViewUIPosition(Curve2DKeyUI* ui)
{
	Point<int> p = getPosInView(ui->item->viewUIPosition->getPoint());
	juce::Rectangle<int> pr = juce::Rectangle<int>(0, 0, 20, 20).withCentre(p);
	if (ui->item->easing != nullptr) pr = pr.getUnion(getBoundsInView(ui->item->easing->getBounds(true)).expanded(5, 5));
	if (ui->item->nextKey != nullptr && (pr.getWidth() == 20 && pr.getHeight() == 20))
	{
		//juce::Rectangle<float> t = ui->item->easing->getBounds(true);
		//juce::Rectangle<int> vt = getBoundsInView(ui->item->easing->getBounds(true));
		//DBG("Weird");
	}
	pr.expand(5, 5);
	ui->setBounds(pr);
	ui->setValueBounds(getViewBounds(pr));
}

void Curve2DUI::updateHandlesForUI(Curve2DKeyUI* ui, bool checkSideItems)
{
	if (ui == nullptr) return;

	int index = itemsUI.indexOf(ui);
	if (checkSideItems)
	{
		if (index > 0)  updateHandlesForUI(itemsUI[index - 1], false);
		if (index < itemsUI.size() - 1)  updateHandlesForUI(itemsUI[index + 1], false);
	}

	bool curSelected = ui->item->isThisOrChildSelected();
	if (curSelected)
	{
		ui->setShowEasingHandles(true, !ui->item->isSelected);
		return;
	}

	bool prevSelected = false;
	if (index > 0 && itemsUI[index - 1] != nullptr)
	{
		Curve2DKey* prevItem = itemsUI[index - 1]->item;
		prevSelected = prevItem->isThisOrChildSelected() && !prevItem->isSelected; //we only want to show if easing is selected only easing
	}
	bool nextSelected = index < itemsUI.size() && itemsUI[index + 1] != nullptr && itemsUI[index + 1]->item->isThisOrChildSelected();

	ui->setShowEasingHandles(prevSelected, nextSelected);

}

bool Curve2DUI::checkItemShouldBeVisible(Curve2DKeyUI* ui)
{
	float focus = manager->focusRange->floatValue();
	if (focus == 0)
	{
		ui->setFocus(0, 0);
		return true;
	}


	float curvePos = manager->position->floatValue() * manager->length->floatValue();

	float minPos = curvePos - focus / 2;
	float maxPos = curvePos + focus / 2;

	bool visible = (ui->item->nextKey != nullptr && ui->item->nextKey->curvePosition >= minPos) && ui->item->curvePosition <= maxPos;

	if (visible)
	{
		ui->setFocus(curvePos - ui->item->curvePosition, focus);
	}
	else
	{
		ui->setShowEasingHandles(false, false);
	}

	return visible;
}

void Curve2DUI::addItemUIInternal(Curve2DKeyUI* ui)
{
	BaseManagerViewUI::addItemUIInternal(ui);
	ui->addMouseListener(this, true);
	ui->item->addAsyncKeyListener(this);
	ui->addKeyUIListener(this);
}

void Curve2DUI::removeItemUIInternal(Curve2DKeyUI* ui)
{
	BaseManagerViewUI::removeItemUIInternal(ui);

	ui->removeMouseListener(this);
	if (!ui->inspectable.wasObjectDeleted())
	{
		ui->item->removeAsyncKeyListener(this);
		ui->removeKeyUIListener(this);
	}
}

void Curve2DUI::mouseDown(const MouseEvent& e)
{
	if (e.eventComponent == this && e.mods.isLeftButtonDown() && e.mods.isCommandDown() && e.mods.isShiftDown())
	{
		paintingMode = true;
		paintingPoints.clear();
		paintingPoints.add(getViewPos(e.getPosition()));
	}
	else
	{
		BaseManagerViewUI::mouseDown(e);
	}
}

void Curve2DUI::mouseDrag(const MouseEvent& e)
{
	if (Curve2DKeyHandle* handle = dynamic_cast<Curve2DKeyHandle*>(e.eventComponent))
	{
		/* Curve2DKey * k = handle->key;
		 int index = manager->items.indexOf(k);

		 Point<float> offset = getViewOffset(e.getEventRelativeTo(this).getOffsetFromDragStart());
		 offset.setY(-offset.y);


		 if (k->nextKey != nullptr) offset.setX(jmin(offset.x, k->nextKey->viewUIPosition->floatValue() - k->movePositionReference.x));
		 if (index > 0) offset.setX(jmax(offset.x, manager->items[index - 1]->viewUIPosition->floatValue() - k->movePositionReference.x));


		 if (e.mods.isShiftDown()) offset.setY(0);
		 if (e.mods.isAltDown()) k->scalePosition(offset, true);
		 else k->movePosition(offset, true);*/
		BaseManagerViewUI::mouseDrag(e);
	}
	else if (e.eventComponent == this)
	{
		if (paintingMode)
		{
			paintingPoints.add(getViewPos(e.getPosition()));
			repaint();
		}
		else
		{
			BaseManagerViewUI::mouseDrag(e);
		}
	}
}

void Curve2DUI::mouseUp(const MouseEvent& e)
{
	if (paintingMode)
	{
		manager->addFromPointsAndSimplify(paintingPoints);

		paintingMode = false;
		paintingPoints.clear();
		repaint();
	}
	else
	{
		BaseManagerViewUI::mouseUp(e);
	}
}

void Curve2DUI::mouseDoubleClick(const MouseEvent& e)
{
	if (e.eventComponent == this)
	{
		Point<float> p = getViewMousePosition();
		Curve2DKey* k = manager->createItem();
		k->viewUIPosition->setPoint(p);
		manager->addItem(k);
	}
	else if (Easing2DUI* eui = dynamic_cast<Easing2DUI*>(e.eventComponent))
	{
		Point<float> p = eui->easing->getClosestPointForPos(getViewMousePosition());
		Curve2DKeyUI* kui = dynamic_cast<Curve2DKeyUI*>(eui->getParentComponent());

		Curve2DKey* k = manager->createItem();
		k->viewUIPosition->setPoint(p);
		k->easingType->setValueWithData(kui->item->easingType->getValueData());

		Array<Point<float>> controlPoints;
		if (eui->easing->type == Easing::BEZIER)
		{
			CubicEasing2D* ce1 = (CubicEasing2D*)eui->easing.get();
			controlPoints = ce1->getSplitControlPoints(p);
		}

		var params(new DynamicObject());
		params.getDynamicObject()->setProperty("index", itemsUI.indexOf(kui) + 1);
		manager->addItem(k, params);

		if (eui->easing->type == Easing::BEZIER)
		{
			CubicEasing2D* ce1 = (CubicEasing2D*)eui->easing.get();
			CubicEasing2D* ce2 = (CubicEasing2D*)k->easing.get();

			ce1->anchor1->setPoint(controlPoints[0] - ce1->start);
			ce1->anchor2->setPoint(controlPoints[1] - ce1->end);

			ce2->anchor1->setPoint(controlPoints[2] - ce2->start);
			ce2->anchor2->setPoint(controlPoints[3] - ce2->end);
		}
	}
}

Component* Curve2DUI::getSelectableComponentForItemUI(Curve2DKeyUI* ui)
{
	return &ui->handle;
}

void Curve2DUI::newMessage(const Curve2DKey::Curve2DKeyEvent& e)
{
	switch (e.type)
	{
	case Curve2DKey::Curve2DKeyEvent::KEY_UPDATED:
	{
		updateViewUIPosition(getUIForItem(e.key));
	}
	break;

	case Curve2DKey::Curve2DKeyEvent::SELECTION_CHANGED:
	{
		updateHandlesForUI(getUIForItem(e.key), true);
	}
	break;
	}
}

void Curve2DUI::newMessage(const ContainerAsyncEvent& e)
{
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		if (e.targetControllable.wasObjectDeleted()) return;
		if (e.targetControllable == manager->value)
		{
			repaint();
		}
		else if (e.targetControllable == manager->focusRange || e.targetControllable == manager->position)
		{
			updateItemsVisibility();
			repaint();
		}
	}
}

void Curve2DUI::newMessage(const AutomationRecorder::RecorderEvent& e)
{
	if (e.type == AutomationRecorder::RecorderEvent::RECORDER_UPDATED) repaint();
}

void Curve2DUI::keyEasingHandleMoved(Curve2DKeyUI* ui, bool syncOtherHandle, bool isFirst)
{
	if (syncOtherHandle)
	{
		int index = itemsUI.indexOf(ui);
		if (isFirst)
		{
			if (index > 0)
			{
				if (itemsUI[index - 1]->item->easingType->getValueDataAsEnum<Easing2D::Type>() == Easing2D::BEZIER)
				{
					if (CubicEasing2D* ce = dynamic_cast<CubicEasing2D*>(itemsUI[index - 1]->item->easing.get()))
					{
						CubicEasing2D* e = dynamic_cast<CubicEasing2D*>(ui->item->easing.get());
						ce->anchor2->setPoint(-e->anchor1->getPoint());
					}
				}
			}
		}
		else
		{
			if (index < itemsUI.size() - 2)
			{
				if (itemsUI[index + 1]->item->easingType->getValueDataAsEnum<Easing2D::Type>() == Easing2D::BEZIER)
				{
					if (CubicEasing2D* ce = dynamic_cast<CubicEasing2D*>(itemsUI[index + 1]->item->easing.get()))
					{
						CubicEasing2D* e = dynamic_cast<CubicEasing2D*>(ui->item->easing.get());
						ce->anchor1->setPoint(-e->anchor2->getPoint());
					}
				}
			}
		}
	}
}

