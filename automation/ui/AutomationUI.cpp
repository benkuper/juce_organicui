/*
  ==============================================================================

	AutomationUI.cpp
	Created: 21 Mar 2020 4:06:30pm
	Author:  bkupe

  ==============================================================================
*/

AutomationUIKeys::AutomationUIKeys(Automation* manager, AutomationUI* _autoUI) :
	autoUI(_autoUI),
	BaseManagerUI(manager->niceName, manager, false),
	autoAdaptViewRange(false),
	paintingMode(false),
	previewMode(false)
{
	//autoSelectWithChildRespect = false;
	resizeOnChildBoundsChanged = false;
	setShowAddButton(false);

	animateItemOnAdd = false;
	manager->addAsyncContainerListener(this);
	manager->addAsyncAutomationListener(this);

	
	setBufferedToImage(true);
	transparentBG = true;

	setViewRange(0, manager->length->floatValue());

	addExistingItems(false);
	
	setInterceptsMouseClicks(true, true);
}

AutomationUIKeys::~AutomationUIKeys()
{
	if (!inspectable.wasObjectDeleted())
	{
		manager->removeAsyncContainerListener(this);
		manager->removeAsyncAutomationListener(this);
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

void AutomationUIKeys::paint(Graphics& g)
{
	if (getWidth() == 0 || !isShowing()) return;

	if (previewMode)
	{
		Path p;
		p.startNewSubPath(Point<float>(0, getYForValue(manager->getValueAtPosition(getPosForX(0)))));
		for (int i = 1; i < getWidth(); i += 2)
		{
			p.lineTo(Point<float>(i, getYForValue(manager->getValueAtPosition(getPosForX(i)))));
		}

		g.setColour(NORMAL_COLOR);
		g.strokePath(p, PathStrokeType(1));
		return;
	}

	if (manager->items.size() > 0)
	{
		if (manager->items[0] != nullptr && manager->items[0]->position->floatValue() > 0)
		{
			g.setColour(NORMAL_COLOR);
			Point<int> p = getPosInView(manager->items[0]->getPosAndValue());
			const float dashLengths[] = { 5, 5 };
			g.drawDashedLine(Line<int>(Point<int>(0, p.y), p).toFloat(), dashLengths, 2);
		}

		if (manager->items[manager->items.size() - 1] != nullptr && manager->items[manager->items.size() - 1]->position->floatValue() < manager->length->floatValue())
		{
			g.setColour(NORMAL_COLOR);
			Point<int> p = getPosInView(manager->items[manager->items.size() - 1]->getPosAndValue());
			const float dashLengths[] = { 5, 5 };
			g.drawDashedLine(Line<int>(p, Point<int>(getWidth(), p.y)).toFloat(), dashLengths, 2);
		}
	}
}

void AutomationUIKeys::paintBackground(juce::Graphics& g)
{
	if (getWidth() == 0) return;

	if (!transparentBG) g.fillAll(bgColor);

	if (previewMode) return;

	if (autoUI->showNumberLines) drawLinesBackground(g);
}

void AutomationUIKeys::drawLinesBackground(Graphics& g)
{
	float start = manager->viewValueRange->x;
	float end = manager->viewValueRange->y;

	if (end <= start) return;

	int minGap = 10;
	int fadeGap = 30;
	const int numDecUnitPerUnit = 10;

	float decimalGap = (getHeight() / (end - start)) / numDecUnitPerUnit;
	float unitGap = (getHeight() / (end - start));

	bool showSeconds = unitGap > minGap;

	float decimalSteps = 1;
	float unitSteps = 1;

	if (showSeconds)
	{
		while (decimalGap < minGap)
		{
			if (decimalSteps == 1) decimalSteps *= 2.5f;
			else decimalSteps *= 2;
			decimalGap = (getHeight() / (end - start)) * decimalSteps / numDecUnitPerUnit;
		}
	}

	while (unitGap < minGap)
	{
		unitSteps *= 2;
		unitGap = ((getHeight() / (end - start))) * unitSteps;
	}

	int unitStart = floor((start / unitSteps)) * unitSteps;
	int unitEndTime = jmax<int>(ceil((end / unitSteps)) * unitSteps, unitStart + 1);

	g.setFont(12);
	float unitFadeAlpha = jlimit<float>(0, 1, jmap<float>(unitGap, minGap, fadeGap, 0, 1));
	float fadeAlpha = jlimit<float>(0, 1, jmap<float>(decimalGap, minGap, fadeGap, 0, 1));

	int uIndex = 0;
	for (float i = unitStart; i <= unitEndTime; i += unitSteps)
	{
		int mty = getYForValue(i);

		if (mty >= 0 && mty <= getHeight())
		{
			//Draw minute
			float alpha = 1;
			if (uIndex % 2 == 0) alpha = unitFadeAlpha;

			g.setColour(BG_COLOR.brighter(.3f).withAlpha(alpha * .4f));
			//g.drawLine(tx, 0, tx, getHeight(), 1);
			g.drawHorizontalLine(mty, 30, (float)getWidth());
			//g.setColour(BG_COLOR.darker(.6f));
			//g.drawRoundedRectangle(getLocalBounds().toFloat(), 2, 2);

			//g.setColour(BG_COLOR.brighter(.7f));
			//g.fillRoundedRectangle( 0, mty - 10, 20, 14, 2);
			g.setColour(BG_COLOR.brighter(.2f).withAlpha(alpha * .5f));
			g.drawText(String(i, 2), 2, mty - 10, 35, 14, Justification::left);

		}

		uIndex++;

		if (showSeconds)
		{
			int sIndex = 0;
			for (float s = decimalSteps; s < numDecUnitPerUnit && i + s / numDecUnitPerUnit <= end; s += decimalSteps)
			{
				int sty = getYForValue(i + s * 1.0f / numDecUnitPerUnit);
				if (sty >= 0 && sty <= getHeight())
				{
					float alpha = 1;
					if (sIndex % 2 == 0) alpha = fadeAlpha;
					g.setColour(BG_COLOR.brighter(.2f).withAlpha(alpha * .2f));
					//g.drawLine(tx, 0, tx, getHeight(), 1);
					g.drawHorizontalLine(sty, 30, (float)getWidth());
					g.setColour(BG_COLOR.brighter(.2f).withAlpha(alpha));
					g.drawText(String(s / numDecUnitPerUnit, 2), 2, sty - 10, 35, 14, Justification::left);

				}

				sIndex++;

			}
		}
	}
}

void AutomationUIKeys::paintOverlay(juce::Graphics& g)
{
	if (previewMode) return;

	if(!autoUI->disableOverlayFill){
		g.setColour(Colours::white.withAlpha(.04f));
		float ty = autoUI->overlayStartY;
		float t0 = getYForValue(0);
		g.fillRect(getLocalBounds().withTop(jmin(ty, t0)).withBottom(jmax(ty, t0)));
	}

	//recorder
	bool interactiveMode = !manager->interactiveSimplifiedPoints.isEmpty();

	if (manager->recorder != nullptr)
	{
		if (manager->recorder->isRecording->boolValue())
		{
			int numRKeys = manager->recorder->keys.size();
			if (numRKeys > 0)
			{
				if (!interactiveMode)
				{
					g.setColour(Colours::red.withAlpha(.2f));
					g.fillRect(getLocalBounds().withLeft(getXForPos(manager->recorder->keys[0].time)).withRight(getXForPos(manager->position->floatValue())));
				}

				if (numRKeys >= 2)
				{
					Path p;
					Point<float> k0(manager->recorder->keys[0].time, manager->recorder->keys[0].value);
					p.startNewSubPath(getPosInView(k0).toFloat());
					for (int i = 1; i < numRKeys; ++i)
					{
						Point<float> ki(manager->recorder->keys[i].time, manager->recorder->keys[i].value);
						p.lineTo(getPosInView(ki).toFloat());
					}

					g.setColour(Colours::orangered);
					g.strokePath(p, PathStrokeType(interactiveMode ? 1 : 2));
				}
			}
		}
	}

	if (interactiveMode)
	{
		if (manager->interactiveSimplifiedPoints.size() >= 2)
		{
			Path p;
			Point<float> k0(manager->interactiveSimplifiedPoints[0].x, manager->interactiveSimplifiedPoints[0].y);
			p.startNewSubPath(getPosInView(k0).toFloat());
			for (int i = 1; i < manager->interactiveSimplifiedPoints.size(); ++i)
			{
				Point<float> ki(manager->interactiveSimplifiedPoints[i].x, manager->interactiveSimplifiedPoints[i].y);
				p.lineTo(getPosInView(ki).toFloat());
			}

			g.setColour(GREEN_COLOR);
			g.strokePath(p, PathStrokeType(2));
		}
	}

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
}

void AutomationUIKeys::resized()
{
	if (previewMode) return;

	updateItemsVisibility();

	for (auto& kui : itemsUI) placeKeyUI(kui);

	if (interactiveSimplificationUI != nullptr)
	{
		juce::Rectangle<int> r = getLocalBounds().removeFromTop(24).withSizeKeepingCentre(300, 24).reduced(2);
		validInteractiveBT->setBounds(r.removeFromRight(80));
		r.removeFromRight(8);
		interactiveSimplificationUI->setBounds(r);
	}
}

void AutomationUIKeys::placeKeyUI(AutomationKeyUI* ui)
{
	if (ui == nullptr || !ui->isVisible()) return;

	juce::Rectangle<int> r = getLocalBounds();

	Point<int> p = getPosInView(ui->item->getPosAndValue());
	juce::Rectangle<int> pr = juce::Rectangle<int>(0, 0, 20, 20).withCentre(p);
	if (ui->item->easing != nullptr)
	{
		juce::Rectangle<int> er = getBoundsInView(ui->item->easing->getBounds(true));
		er.setSize(jmax(er.getWidth(), er.getWidth() + 1), jmax(er.getHeight(), er.getHeight() + 1));
		pr = pr.getUnion(er);
	}

	pr = pr.expanded(5, 5).getIntersection(r).withY(0).withBottom(getHeight());
	ui->setBounds(pr);
	ui->setValueBounds(getViewBounds(pr));

}

void AutomationUIKeys::updateHandlesForUI(AutomationKeyUI* ui, bool checkSideItems)
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
		AutomationKey* prevItem = itemsUI[index - 1]->item;
		prevSelected = prevItem->isThisOrChildSelected() && !prevItem->isSelected; //we only want to show if easing is selected only easing
	}
	bool nextSelected = index < itemsUI.size() && itemsUI[index + 1] != nullptr && itemsUI[index + 1]->item->isThisOrChildSelected();

	ui->setShowEasingHandles(prevSelected, nextSelected);

}

void AutomationUIKeys::setPreviewMode(bool value)
{
	previewMode = value;
	if (previewMode)
	{
		for (auto& kui : itemsUI) kui->setVisible(false);
	}
	
	resized();
	autoUI->setRepaint(true, true, true);
}

void AutomationUIKeys::setViewRange(float start, float end)
{
	if(viewPosRange.getX() == start && viewPosRange.getY() == end) return;
	viewPosRange.setXY(start, end);
	viewLength = viewPosRange.y - viewPosRange.x;

	resized();
	autoUI->setRepaint(true, true, false);
}

void AutomationUIKeys::updateItemsVisibility()
{
	if (itemsUI.size() == 0) return;
	if (previewMode) return;

	int firstIndex = jmax(manager->items.indexOf(manager->getKeyForPosition(viewPosRange.x)), 0);
	int lastIndex = jmax(manager->items.indexOf(manager->getKeyForPosition(viewPosRange.y)) + 1, firstIndex);

	for (int i = 0; i < itemsUI.size(); ++i)
	{
		itemsUI[i]->setVisible(i >= firstIndex && i <= lastIndex);
	}
}

void AutomationUIKeys::addItemUIInternal(AutomationKeyUI* ui)
{
	ui->addMouseListener(this, true);
	ui->item->addAsyncKeyListener(this);
	ui->addKeyUIListener(this);
}

void AutomationUIKeys::removeItemUIInternal(AutomationKeyUI* ui)
{
	ui->removeMouseListener(this);
	if (!ui->inspectable.wasObjectDeleted())
	{
		ui->item->removeAsyncKeyListener(this);
		ui->removeKeyUIListener(this);
	}
}

void AutomationUIKeys::mouseDown(const MouseEvent& e)
{
	if (e.eventComponent == this)
	{
		if (e.mods.isLeftButtonDown() && e.mods.isCommandDown() && e.mods.isShiftDown())
		{
			paintingMode = true;
			paintingPoints.clear();
			paintingPoints.add(getViewPos(e.getPosition()));
			lastPaintingPoint = paintingPoints[0];
		}
		else if (e.mods.isAltDown())
		{
			viewValueRangeAtMouseDown = manager->viewValueRange->getPoint();
		}
		else
		{
			if (!e.mods.isRightButtonDown() || autoUI->showMenuOnRightClick)
			{
				BaseManagerUI::mouseDown(e);
			}
		}
	}
	else if (AutomationKeyHandle* handle = dynamic_cast<AutomationKeyHandle*>(e.eventComponent))
	{
		snapTimes.clear();
		if (autoUI->getSnapTimesFunc != nullptr) autoUI->getSnapTimesFunc(&snapTimes, handle->key);
		//snapTimes.removeAllInstancesOf(handle->key->position->floatValue());
	}
}

void AutomationUIKeys::mouseDrag(const MouseEvent& e)
{
	if (AutomationKeyHandle* handle = dynamic_cast<AutomationKeyHandle*>(e.eventComponent))
	{
		AutomationKey* k = handle->key;
		int index = manager->items.indexOf(k);

		Point<float> offset = getViewPos(e.getEventRelativeTo(this).getOffsetFromDragStart(), true);
		offset.setY(-offset.y);

		if (k->nextKey != nullptr) offset.setX(jmin(offset.x, k->nextKey->position->floatValue() - k->movePositionReference.x));
		if (index > 0) offset.setX(jmax(offset.x, manager->items[index - 1]->position->floatValue() - k->movePositionReference.x));


		if (e.mods.isShiftDown()) offset.setY(0);
		else if (e.mods.isAltDown()) offset.setX(0);//k->scalePosition(offset, true);

		if (e.mods.isLeftButtonDown())
		{
			if (e.mods.isShiftDown() && e.mods.isAltDown())
			{
				float targetTime = k->movePositionReference.x + offset.x;
				float diff = INT32_MAX;
				float tTime = targetTime;
				for (auto& t : snapTimes)
				{
					float d = fabsf(tTime - t);
					if (d < diff)
					{
						diff = d;
						targetTime = t;
					}
				}

				offset.x = targetTime - k->movePositionReference.x;

			}

			k->movePosition(offset, true);
		}

		else if (e.mods.isRightButtonDown()) k->scalePosition(offset, true);
	}
	else if (e.eventComponent == this)
	{
		if (paintingMode)
		{
			Point<float> newPoint = getViewPos(e.getPosition());

			float minX = jmin(newPoint.x, lastPaintingPoint.x);
			float maxX = jmax(newPoint.x, lastPaintingPoint.x);

			int inKeyStart = -1;
			int inKeyEnd = -1;
			int indexBeforeNewPoint = 0;
			int indexAfterNewPoint = paintingPoints.size();

			for (int i = 0; i < paintingPoints.size(); ++i)
			{
				Point<float> p = paintingPoints[i];
				if (p.x >= newPoint.x) indexAfterNewPoint = jmin(indexAfterNewPoint, i);
				else indexBeforeNewPoint = jmax(indexBeforeNewPoint, i);

				if (p.x == lastPaintingPoint.x) continue;

				if (p.x >= minX && p.x <= maxX)
				{
					inKeyStart = inKeyStart == -1 ? i : jmin(i, inKeyStart);
					inKeyEnd = jmax(inKeyEnd, i);
				}
			}

			bool foundKeysToRemove = inKeyStart != -1 && inKeyEnd != -1;
			if (foundKeysToRemove)
			{
				paintingPoints.removeRange(inKeyStart, inKeyEnd - inKeyStart + 1);
			}

			paintingPoints.insert(indexAfterNewPoint, newPoint);
			lastPaintingPoint.setXY(newPoint.x, newPoint.y);
			repaint();
		}
		else if (e.mods.isAltDown())
		{
			//if (!manager->valueRange->enabled)
			//{
				if (e.mods.isShiftDown())
				{
					float pRel = 1 - (e.getMouseDownPosition().y * 1.0f / getHeight());
					float rangeAtMouseDown = (viewValueRangeAtMouseDown.y - viewValueRangeAtMouseDown.x);
					float tRange = rangeAtMouseDown + (e.getOffsetFromDragStart().y / 10.0f);
					if (tRange > .2f)
					{
						float rangeDiff = tRange - rangeAtMouseDown;
						float tMin = viewValueRangeAtMouseDown.x - rangeDiff * pRel;
						float tMax = viewValueRangeAtMouseDown.y + rangeDiff * (1 - pRel);
						manager->viewValueRange->setPoint(tMin, tMax);
					}
				}
				else
				{
					float deltaVal = getValueForY(e.getDistanceFromDragStartY(), true);
					manager->viewValueRange->setPoint(viewValueRangeAtMouseDown.x + deltaVal, viewValueRangeAtMouseDown.y + deltaVal);
				}
			//}
		}
		{
			BaseManagerUI::mouseDrag(e);
		}
	}
}

void AutomationUIKeys::mouseUp(const MouseEvent& e)
{
	if (paintingMode)
	{
		manager->addFromPointsAndSimplifyBezier(paintingPoints); //always use bezier for drawing
		paintingMode = false;
		paintingPoints.clear();
		repaint();
	}
	else
	{
		BaseManagerUI::mouseUp(e);
	}
}

void AutomationUIKeys::mouseDoubleClick(const MouseEvent& e)
{
	if (e.eventComponent == this)
	{
		Point<float> p = getViewPos(e.getPosition());
		manager->addKey(p.x, p.y, true);
	}
	else if (EasingUI* eui = dynamic_cast<EasingUI*>(e.eventComponent))
	{
		float pos = getPosForX(e.getEventRelativeTo(this).getPosition().x);
		manager->insertKeyAt(pos);
	}
}

void AutomationUIKeys::addItemFromMenu(AutomationKey* k, bool fromAddbutton, Point<int> pos)
{
	manager->addKey(getPosForX(pos.x), getValueForY(pos.y), true);
}

void AutomationUIKeys::addMenuExtraItems(PopupMenu& p, int startIndex)
{
	//p.addSeparator();
	PopupMenu em;
	int index = 0;
	for (auto& e : Easing::typeNames)
	{
		em.addItem(startIndex + (index++), e);
	}
	p.addSubMenu("Change all easings", em);
}

void AutomationUIKeys::handleMenuExtraItemsResult(int result, int startIndex)
{
	String typeName = Easing::typeNames[result - startIndex];

	Array<UndoableAction*> actions;
	for (auto& i : manager->items)
	{
		actions.add(i->easingType->setUndoableValue(i->easingType->value, typeName, true));
	}

	UndoMaster::getInstance()->performActions("Change all easings", actions);
}

Component* AutomationUIKeys::getSelectableComponentForItemUI(AutomationKeyUI* ui)
{
	return &ui->handle;
}

Point<float> AutomationUIKeys::getViewPos(Point<int> pos, bool relative)
{
	return Point<float>(getPosForX(pos.x, relative), getValueForY(pos.y, relative));
}

juce::Rectangle<float> AutomationUIKeys::getViewBounds(juce::Rectangle<int> pos, bool relative)
{
	juce::Rectangle<float> r = juce::Rectangle<float>(getViewPos(pos.getBottomLeft()), getViewPos(pos.getTopRight()));
	if (relative) r.setPosition(0, 0);
	return r;
}

Point<int> AutomationUIKeys::getPosInView(Point<float> pos, bool relative)
{
	return Point<int>(getXForPos(pos.x, relative), getYForValue(pos.y, relative));
}

juce::Rectangle<int> AutomationUIKeys::getBoundsInView(juce::Rectangle<float> pos, bool relative)
{
	juce::Rectangle<int> r = juce::Rectangle<int>(getPosInView(pos.getTopLeft()), getPosInView(pos.getBottomRight()));
	if (relative) r.setPosition(0, 0);
	return r;
}

float AutomationUIKeys::getPosForX(int x, bool relative)
{
	float rel = (x * 1.0f / getWidth()) * viewLength;
	return relative ? rel : viewPosRange.x + rel;
}

int AutomationUIKeys::getXForPos(float x, bool relative)
{
	return ((relative ? x : x - viewPosRange.x) / viewLength) * getWidth();
}

float AutomationUIKeys::getValueForY(int y, bool relative)
{
	float valRange = (manager->viewValueRange->y - manager->viewValueRange->x);
	float rel = (1 - y * 1.0f / getHeight());
	return relative ? (1 - rel) * valRange : manager->viewValueRange->x + rel * valRange;
}

int AutomationUIKeys::getYForValue(float x, bool relative)
{
	return (1 - (relative ? x : x - manager->viewValueRange->x) / (manager->viewValueRange->y - manager->viewValueRange->x)) * getHeight();
}

void AutomationUIKeys::newMessage(const AutomationKey::AutomationKeyEvent& e)
{
	switch (e.type)
	{
	case AutomationKey::AutomationKeyEvent::KEY_UPDATED:
	{
		placeKeyUI(getUIForItem(e.key));
	}
	break;

	case AutomationKey::AutomationKeyEvent::SELECTION_CHANGED:
	{
		updateHandlesForUI(getUIForItem(e.key), true);
	}
	break;
	}
}

void AutomationUIKeys::newMessage(const ContainerAsyncEvent& e)
{
	if (e.targetControllable == nullptr || e.targetControllable.wasObjectDeleted() || inspectable.wasObjectDeleted()) return;
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		if (e.targetControllable == manager->value || e.targetControllable == manager->position)
		{
			bool refreshOverlay = !manager->interactiveSimplifiedPoints.isEmpty() || (manager->recorder != nullptr && manager->recorder->isRecording->boolValue()) || (paintingMode && paintingPoints.size() > 0);

			autoUI->setRepaint(false,refreshOverlay,false);
		}
		if (manager->items.size() > 0 && (e.targetControllable->parentContainer == manager->items[0] || e.targetControllable->parentContainer == manager->items[manager->items.size() - 1]))
		{
			autoUI->setRepaint(true, false, false);
		}
		else if (e.targetControllable == manager->viewValueRange)
		{
			resized();
			autoUI->setRepaint(true, true, true);
		}
		else if (e.targetControllable == manager->length)
		{
			if (autoAdaptViewRange)
			{
				setViewRange(0, manager->length->floatValue());
			}
		}
	}
}

void AutomationUIKeys::newMessage(const Automation::AutomationEvent& e)
{
	if (e.type == e.INTERACTIVE_SIMPLIFICATION_CHANGED)
	{
		bool interactiveMode = !manager->interactiveSourcePoints.isEmpty();
		if (interactiveMode)
		{
			if (interactiveSimplificationUI == nullptr)
			{
				interactiveSimplificationUI.reset(manager->recorder->simplificationTolerance->createSlider());
				validInteractiveBT.reset(new TextButton("Validate"));

				validInteractiveBT->addListener(this);

				addAndMakeVisible(interactiveSimplificationUI.get());
				addAndMakeVisible(validInteractiveBT.get());
			}
		}
		else
		{
			if (interactiveSimplificationUI != nullptr)
			{
				removeChildComponent(interactiveSimplificationUI.get());
				removeChildComponent(validInteractiveBT.get());

				interactiveSimplificationUI.reset();
				validInteractiveBT.reset();
			}
		}

		resized();
		repaint();
	}
}

void AutomationUIKeys::keyEasingHandleMoved(AutomationKeyUI* ui, bool syncOtherHandle, bool isFirst)
{
	if (syncOtherHandle)
	{
		int index = itemsUI.indexOf(ui);
		if (isFirst)
		{
			if (index > 0)
			{
				if (itemsUI[index - 1]->item->easingType->getValueDataAsEnum<Easing::Type>() == Easing::BEZIER)
				{
					if (CubicEasing* ce = dynamic_cast<CubicEasing*>(itemsUI[index - 1]->item->easing.get()))
					{
						CubicEasing* e = dynamic_cast<CubicEasing*>(ui->item->easing.get());
						ce->anchor2->setPoint(-e->anchor1->getPoint());
					}
				}
			}
		}
		else
		{
			if (index < itemsUI.size() - 2)
			{
				if (itemsUI[index + 1]->item->easingType->getValueDataAsEnum<Easing::Type>() == Easing::BEZIER)
				{
					if (CubicEasing* ce = dynamic_cast<CubicEasing*>(itemsUI[index + 1]->item->easing.get()))
					{
						CubicEasing* e = dynamic_cast<CubicEasing*>(ui->item->easing.get());
						ce->anchor1->setPoint(-e->anchor2->getPoint());
					}
				}
			}
		}
	}
}

void AutomationUIKeys::buttonClicked(Button* b)
{
	if (b == validInteractiveBT.get())
	{
		manager->finishInteractiveSimplification();
	}
	else
	{
		BaseManagerUI::buttonClicked(b);
	}
}



AutomationUILayer::AutomationUILayer(AutomationUI* _ui, int _id) :
	ui(_ui),
	id(_id)
{
	setInterceptsMouseClicks(false,false);
	if(id == 1)
		setBufferedToImage(true);
}

void AutomationUILayer::paint(juce::Graphics& g)
{
	if(!isShowing()) return;

	if(id == 0){
		ui->keysUI.paintOverlay(g);
	}else if(id == 1){
		ui->keysUI.paintBackground(g);
	}else{
		//draw current point
		g.setColour(BLUE_COLOR);
		g.fillEllipse(juce::Rectangle<float>(0, 0, 6, 6));
	}
	//ui->validatePaint();
}

void AutomationUILayer::resized()
{
}




AutomationUI::AutomationUI(Automation* manager) :
	UITimerTarget(ORGANICUI_DEFAULT_TIMER, "AutomationUI", true),
	keysUI(manager, this),
	overlay(this, 0),
	background(this, 1),
	cursor(this, 2),
	showNumberLines(true),
	showMenuOnRightClick(true),
	lastRepaintOverlayPoint(-1,-1),
	overlayStartY(0)
{
	addAndMakeVisible(&background);
	addAndMakeVisible(&keysUI);
	addAndMakeVisible(&overlay);
	addAndMakeVisible(&cursor);

	setSize(100, 100);

	setInterceptsMouseClicks(false, true);
}

void AutomationUI::paint(juce::Graphics& g)
{
}

void AutomationUI::resized()
{
	shouldResize = true;
	shouldRepaint = true;
}

void AutomationUI::setRepaint(bool _keys, bool _overlay, bool _background)
{
	if(_background) background.repaint();

	shouldRepaint = true;
	shouldRepaintKeys = _keys;
	shouldRepaintOverlay = _overlay;
}

void AutomationUI::paintOverChildren(Graphics& g)
{
	validatePaint();
}

void AutomationUI::handlePaintTimerInternal()
{
	bool display = isShowing();

	int lastOverlayY = lastRepaintOverlayPoint.getY();
	overlayStartY = keysUI.getYForValue(keysUI.manager->value->floatValue());
	int newXpos = keysUI.getXForPos(keysUI.manager->position->floatValue());

	if(overlayStartY != lastOverlayY || lastRepaintOverlayPoint.getX() != newXpos){
		cursor.setBounds(newXpos - 3, overlayStartY - 3, 6, 6);

		lastRepaintOverlayPoint.setXY(newXpos, overlayStartY);
	}

	if(shouldResize)
	{
		keysUI.setBounds(getLocalBounds());
		overlay.setBounds(getLocalBounds());
		background.setBounds(getLocalBounds());
		lastRepaintOverlayPoint = Point<int>(0,0);
		shouldResize = false;
		return;
	}

	if(!display) return;

	if(shouldRepaintKeys){
		keysUI.repaint();
		shouldRepaintKeys = false;
	}

	if(shouldRepaintOverlay){
		overlay.repaint();
		shouldRepaintOverlay = false;
		return;
	}

	if(!disableOverlayFill && overlayStartY != lastOverlayY){
		int startX,width,startY,height;
		startX = overlay.getX();
		width = overlay.getWidth();
		startY = jmin(overlayStartY, lastOverlayY);
		height = abs(overlayStartY - lastOverlayY);

		overlay.repaint(startX, startY, width, height);
	}
}
