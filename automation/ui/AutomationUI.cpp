/*  ==============================================================================

	AutomationUI.cpp
	Created: 11 Dec 2016 1:22:02pm
	Author:  Ben

  ==============================================================================
*/


AutomationUI::AutomationUI(Automation* _automation) :
	BaseManagerUI("Automation", _automation, false),
	Thread("AutomationViewGenerator"),
	autoSwitchMode(false),
	firstROIKey(0),
	lastROIKey(0),
	autoResetViewRangeOnLengthUpdate(false),
	currentUI(nullptr)
	//transformer(nullptr)
{

	setMouseClickGrabsKeyboardFocus(true);
	setWantsKeyboardFocus(true);

	manager->selectionManager->addSelectionListener(this);


	setShowAddButton(false);
	animateItemOnAdd = false;

	transparentBG = true;
	setViewRange(0, manager->length->floatValue());
	manager->addAsyncContainerListener(this);

	resizeOnChildBoundsChanged = false;
	addExistingItems();

	noItemText = "Add keys by double-clicking or alt+click here";

	viewMode = autoSwitchMode ? EDIT : VIEW; //force trigger change on setViewMode
	setViewMode(autoSwitchMode ? VIEW : EDIT);

	//updateROI();

	setSize(100, 100);

	startTimerHz(30);
}

AutomationUI::~AutomationUI()
{
	if (!inspectable.wasObjectDeleted() && manager->selectionManager != nullptr)
	{
		manager->selectionManager->removeSelectionListener(this);
		manager->removeAsyncContainerListener(this);
	}

	signalThreadShouldExit();
	stopThread(1000);
}

void AutomationUI::setViewMode(ViewMode mode)
{
	if (viewMode == mode) return;
	viewMode = mode;

	/*if (transformer != nullptr)
	{
		removeChildComponent(transformer.get());
		transformer = nullptr;
	}*/

	switch (viewMode)
	{
	case EDIT:
		signalThreadShouldExit();
		waitForThreadToExit(500);

		updateROI();
		break;

	case VIEW:
	{
		for (auto& i : itemsUI) i->setVisible(false);
		shouldUpdateImage = true;
		startThread();
	}
	break;
	}


	repaint();
}

void AutomationUI::setViewRange(float start, float end)
{
	if (viewStartPos == start && viewEndPos == end) return;

	viewStartPos = start;
	viewEndPos = end;

	updateROI();
	shouldUpdateImage = true;
	shouldRepaint = true;
}

void AutomationUI::updateROI()
{
	if (viewMode != EDIT) return;

	if (itemsUI.size() == 0) return;

	int len = itemsUI.size() - 1;
	for (int i = 0; i < len; i++)
	{
		itemsUI[i]->setVisible(false);
		if (isInView(itemsUI[i]))
		{
			firstROIKey = jmax<int>(i - 1, 0);
			break;
		}
	}

	for (int i = len; i >= 0; i--)
	{
		itemsUI[i]->setVisible(false);
		if (isInView(itemsUI[i]))
		{
			lastROIKey = jmin<int>(i + 1, len);
			break;
		}
	}

	for (int i = firstROIKey; i <= lastROIKey; i++)
	{
		if (itemsUI[i] != nullptr) itemsUI[i]->setVisible(true);
	}

	resized();
}

void AutomationUI::paint(Graphics& g)
{
	BaseManagerUI::paint(g);

	if (inspectable.wasObjectDeleted()) return;

	if (getWidth() == 0 || getHeight() == 0) return;


	float step = .25f;
	float rangeDiff = jmax(manager->viewValueRange->y - manager->viewValueRange->x, .1f);
	float stepY = getHeight() * step / rangeDiff;
	while (stepY < 20)
	{
		step *= 2;
		stepY = getHeight() * step / rangeDiff;
	}

	float bigStep = step >= 1 ? 10 : step >= .5f ? 5 : 1;

	float startY = manager->viewValueRange->x - fmodf(manager->viewValueRange->x, step);
	float endY = manager->viewValueRange->y - fmodf(manager->viewValueRange->x, step);

	g.setFont(12.0);

	for (float i = startY; i <= endY; i += step)
	{
		float thickness = fmodf(i, bigStep) == 0 ? 1 : fmodf(i, bigStep / 2) == 0 ? 1 : .5f;
		float alpha = fmodf(i, 1) == 0 ? .25f : fmodf(i, .5f) == 0 ? .15f : .1f;
		g.setColour(Colours::white.withAlpha(alpha));
		float ty = getYForValue(i);
		float textOffset = ty > getHeight() / 2 ? -12 : 4;
		g.drawText(String(i, 2), Rectangle<int>(0, ty + textOffset, 35, 10), Justification::centred);
		g.drawLine(0, ty, getWidth(), ty, thickness);
	}

	if (viewMode == VIEW)
	{
		imageLock.enter();
		//{
		g.setColour(Colours::white);
		g.drawImage(viewImage, getLocalBounds().toFloat());
		imageLock.exit();
		//}
	}

	//int count = 0;
	if (itemsUI.size() >= 2)
	{
		int y0 = getYForValue(0);
		for (int i = 0; i < manager->numDimensions; i++)
		{
			Colour c = manager->numDimensions == 1 ? Colours::white : Colour::fromHSV(i * .3f, .9f, 1, 0xFF);

			int ty = getYForValue(manager->values[i]->floatValue());

			juce::Rectangle<int> vr(0, jmin(y0, ty), getWidth(), abs(ty - y0));
			g.setColour(c.withAlpha(.1f));
			if (vr.getHeight() != 0) g.fillRect(vr);

			//pos-value feedback
			g.setColour(c.brighter(.7f).withAlpha(.7f));
			g.drawEllipse(juce::Rectangle<int>(0, 0, 6, 6).withCentre(Point<int>(getXForPos(manager->position->floatValue()), ty)).toFloat(), 1);
		}
	}

	//recorder
	if (manager->recorder != nullptr)
	{
		if (manager->recorder->isRecording->boolValue())
		{
			int numRKeys = manager->recorder->keys.size();
			if (numRKeys > 0)
			{
				g.setColour(Colours::red.withAlpha(.3f));
				g.fillRect(getLocalBounds().withLeft(getXForPos(manager->recorder->keys[0].x)).withRight(getXForPos(manager->position->floatValue())));

				if (numRKeys >= 2)
				{
					Path p;
					Point<float> k = manager->recorder->keys[0];
					p.startNewSubPath(getXForPos(k.x), getYForValue(k.x));
					for (int i = 1; i < numRKeys; i++)
					{
						k = manager->recorder->keys[i];
						p.lineTo(getXForPos(k.x), getYForValue(k.y));
					}
					//p.closeSubPath();
					g.setColour(Colours::orangered);
					g.strokePath(p, PathStrokeType(2));
				}
			}
		}
	}
}


void AutomationUI::resized()
{
	if (inspectable.wasObjectDeleted()) return;

	if (viewMode == VIEW)
	{
		shouldUpdateImage = true;
		return;
	}

	MessageManagerLock mm;

	if (getParentComponent() == nullptr) return;
	if (getWidth() == 0 || getHeight() == 0) return;
	if (itemsUI.size() == 0) return;

	for (int i = lastROIKey; i >= firstROIKey; i--)
	{
		placeKeyUI(itemsUI[i], true);
		//itemsUI[i]->toBack(); // place each ui in front of its right : to be better
	}

	//if (transformer != nullptr) transformer->updateBoundsFromKeys();

	shouldRepaint = true;//overkill? needed to have proper value feedback when creating ui and resizing for the first time
}

void AutomationUI::placeKeyUI(AutomationKeyUI* kui, bool placePrevKUI)
{

	int index = itemsUI.indexOf(kui);
	if (kui == nullptr) return;

	int tx = getXForPos(kui->item->position->floatValue());
	//Array<int> ty = getYForKey(kui->item);
	juce::Rectangle<int> kr;

	if (index < itemsUI.size() - 1)
	{
		AutomationKeyUI* nextKey = itemsUI[index + 1];
		int tx2 = getXForPos(nextKey->item->position->floatValue());
		//Array<int> ty2 = getYForKey(nextKey->item);

		//Rectangle<int> kr2 = Rectangle<int>(0, 0, AutomationKeyUI::handleClickZone, AutomationKeyUI::handleClickZone).withCentre(Point<int>(tx2, ty2));
		kr = juce::Rectangle<int>(tx, 0, tx2 - tx, getHeight()).expanded(AutomationKeyUI::handleClickZone / 2, 0);
		//kui->setKeyPositions(ty, ty2);
	}
	else
	{
		kr = juce::Rectangle<int>(0, 0, AutomationKeyUI::handleClickZone, getHeight()).withPosition(tx - AutomationKeyUI::handleClickZone / 2, 0);
		//kui->setKeyPositions(ty, 0);
	}

	kui->setViewValueRange(manager->viewValueRange->getPoint());
	kui->setBounds(kr);

	if (placePrevKUI && index > 0)
	{
		placeKeyUI(itemsUI[index - 1], false);
	}
}

int AutomationUI::getXForPos(float time)
{
	if (viewStartPos == viewEndPos) return 0;
	return (int)jmap<float>(time, viewStartPos, viewEndPos, 0, (float)getWidth());
}

float AutomationUI::getPosForX(int tx, bool offsetStart)
{

	float viewRange = viewEndPos - viewStartPos;
	float mapStart = offsetStart ? viewStartPos : 0;

	if (getWidth() == 0) return mapStart;
	return jmap<float>((float)tx, 0, (float)getWidth(), mapStart, mapStart + viewRange);
}

int AutomationUI::getYForValue(float value)
{
	int result = (int)jmap<float>(value, manager->viewValueRange->x, manager->viewValueRange->y, getHeight() - 1, 0);
	return result;

	//(int)((1 - value) * (getHeight() - 1));
}

Array<int> AutomationUI::getYForKey(AutomationKey* k)
{
	Array<int> result;
	for (auto& v : k->values) result.add(getYForValue(v->floatValue()));
	return result;
}

float AutomationUI::getValueForY(int ty, bool zeroIsBottom, bool relative)
{
	float rel = ty * 1.f / (getHeight() - 1);
	if (zeroIsBottom) rel = 1 - rel;
	if (relative) return rel * (manager->viewValueRange->y - manager->viewValueRange->x);
	float result = jmap<float>(rel, manager->viewValueRange->x, manager->viewValueRange->y);

	return result;
}

bool AutomationUI::isInView(AutomationKeyUI* kui)
{
	return kui->item->position->floatValue() >= viewStartPos && kui->item->position->floatValue() <= viewEndPos;
}

void AutomationUI::homeViewYRange()
{
	manager->viewValueRange->setPoint(0, 1);
}

void AutomationUI::frameViewYRange()
{
	float minY = INT32_MAX;
	float maxY = INT32_MIN;

	for (int i = firstROIKey; i <= lastROIKey; i++)
	{
		for (auto& v : manager->items[i]->values)
		{
			minY = jmin(v->floatValue(), minY);
			maxY = jmax(v->floatValue(), maxY);
		}
	}

	if (minY == maxY)
	{
		minY -= .5f;
		maxY += .5f;
	}
	else
	{
		float dist = maxY - minY;
		minY -= dist * .1f;
		maxY += dist * .1f;
	}

	manager->viewValueRange->setPoint(minY, maxY);

}

AutomationKeyUI* AutomationUI::getClosestKeyUIForPos(float pos, int start, int end)
{
	if (itemsUI.size() == 0) return nullptr;

	if (start == -1) start = 0;
	if (end == -1) end = itemsUI.size() - 1;


	if (pos < itemsUI[0]->item->position->floatValue()) return itemsUI[0];
	if (pos > itemsUI[itemsUI.size() - 1]->item->position->floatValue()) return itemsUI[itemsUI.size() - 1];


	if (end - start <= 1) return itemsUI[start];

	int midIndex = (int)floor((start + end) / 2);
	float medPos = itemsUI[midIndex]->item->position->floatValue();

	if (pos == medPos) return itemsUI[midIndex];

	else if (pos > medPos)
	{
		return getClosestKeyUIForPos(pos, midIndex, end);
	}
	else
	{
		return getClosestKeyUIForPos(pos, start, midIndex);
	}
}

void AutomationUI::itemAddedAsync(AutomationKey* k)
{
	BaseManagerUI::itemAddedAsync(k);
	updateROI();

}

void AutomationUI::itemsReorderedAsync()
{
	BaseManagerUI::itemsReorderedAsync();
	updateROI();
}

AutomationKeyUI* AutomationUI::createUIForItem(AutomationKey* item)
{
	return new AutomationKeyUI(item);
}

void AutomationUI::addItemUIInternal(AutomationKeyUI* kui)
{
	for (auto& h : kui->handles) if (h != nullptr) h->addMouseListener(this, false);
}

void AutomationUI::removeItemUIInternal(AutomationKeyUI* kui)
{
	/*
	if (transformer != nullptr)
	{
		removeChildComponent(transformer.get());
		transformer = nullptr;
	}
	*/

	for (auto& h : kui->handles) if (h != nullptr) h->removeMouseListener(this);
	updateROI();
}

void AutomationUI::mouseDown(const MouseEvent& e)
{
	BaseManagerUI::mouseDown(e);

	if (e.eventComponent == this)
	{
		if (e.mods.isLeftButtonDown())
		{
			if (e.mods.isShiftDown())
			{
				rangeAtMouseDown = manager->viewValueRange->getPoint();
			}
			else if (e.mods.isAltDown())
			{
				float position = getPosForX(e.getPosition().x);
				Array<float> values = manager->getValuesForPosition(position);
				values.set(0, getValueForY(e.getPosition().y));
				manager->addKey(position, values);
				manager->reorderItems();
			}
		}
		else
		{
			/*
			Array<Component *> selectables;
			Array<Inspectable *> inspectables;
			for (auto &i : itemsUI) if (i->isVisible())
			{
				selectables.add(&i->handle);
				inspectables.add(i->inspectable);
			}


			if (transformer != nullptr)
			{
				removeChildComponent(transformer.get());
				transformer = nullptr;
			}


			if (InspectableSelector::getInstance()) InspectableSelector::getInstance()->startSelection(this, selectables, inspectables, manager->selectionManager, !e.mods.isCommandDown() && !e.mods.isShiftDown());
			*/
		}
	}
	else
	{
		if (e.mods.isShiftDown())
		{
			AutomationKeyUI::Handle* kHandle = dynamic_cast<AutomationKeyUI::Handle*>(e.eventComponent);
			if (kHandle != nullptr)
			{
				if (manager->selectionManager->currentInspectables.size() > 0)
				{
					AutomationKey* lastSelectedKey = dynamic_cast<AutomationKey*>(manager->selectionManager->currentInspectables[manager->selectionManager->currentInspectables.size() - 1].get());
					AutomationKey* sKey = ((AutomationKeyUI*)kHandle->getParentComponent())->item;

					int i1 = manager->items.indexOf(lastSelectedKey);
					int i2 = manager->items.indexOf(sKey);

					int index1 = jmin(i1, i2) + 1;
					int index2 = jmax(i1, i2) - 1;

					for (int i = index1; i <= index2; i++)
					{
						manager->items[i]->selectThis(true);
					}
				}
			}
		}

	}

}

void AutomationUI::mouseDoubleClick(const MouseEvent& e)
{
	if (e.eventComponent == this)
	{
		float position = getPosForX(e.getPosition().x);
		Array<float> values = manager->getValuesForPosition(position);
		values.set(0, getValueForY(e.getPosition().y));
		manager->addKey(position, values);
		manager->reorderItems();
	}
}

void AutomationUI::mouseDrag(const MouseEvent& e)
{
	if (e.eventComponent == this)
	{
		if (e.mods.isShiftDown())
		{
			if (e.mods.isAltDown())
			{

				float valOffset = e.getDistanceFromDragStartY() / 100.f;
				float minR = rangeAtMouseDown.x - valOffset ;
				float maxR = jmax<float>(minR + .1f, rangeAtMouseDown.y + valOffset);
				manager->viewValueRange->setPoint(minR, maxR);
			}
			else
			{
				float valOffset = getValueForY(e.getDistanceFromDragStartY(), false, true);
				manager->viewValueRange->setPoint(rangeAtMouseDown.x + valOffset, rangeAtMouseDown.y + valOffset);
			}

		}
	}
	else
	{
		AutomationKeyUI::Handle* h = dynamic_cast<AutomationKeyUI::Handle*>(e.eventComponent);

		if (h != nullptr)
		{

			int index = h->dimensionIndex;

			AutomationKeyUI* kui = static_cast<AutomationKeyUI*>(h->getParentComponent());
			if (e.mods.isLeftButtonDown())
			{
				Point<int> mp = e.getEventRelativeTo(this).getPosition();
				float pos = getPosForX(mp.x);
				float val = getValueForY(mp.y);

				MouseInputSource source = Desktop::getInstance().getMainMouseSource();

				if (e.mods.isShiftDown())
				{
					float initX = getXForPos(kui->posAtMouseDown);
					float initY = getYForValue(kui->valueAtMouseDown);

					if (fabsf(mp.x - initX) > fabsf(mp.y - initY))
					{
						h->setMouseCursor(MouseCursor::LeftRightResizeCursor);
						val = kui->valueAtMouseDown;
					}
					else
					{
						h->setMouseCursor(MouseCursor::UpDownResizeCursor);
						pos = kui->posAtMouseDown;
					}
				}
				else
				{
					h->setMouseCursor(MouseCursor::NormalCursor);
				}


				if (GlobalSettings::getInstance()->constrainKeysToNeighbours->boolValue())
				{
					int itemIndex = manager->items.indexOf(kui->item);
					if (itemIndex > 0) pos = jmax(pos, manager->items[itemIndex - 1]->position->floatValue() + .01f);
					if (itemIndex < manager->items.size() - 1)  pos = jmin(pos, manager->items[itemIndex + 1]->position->floatValue() - .01f);
				}

				kui->item->position->setValue(pos);
				kui->item->values[index]->setValue(val);
			}
		}
	}
}

void AutomationUI::mouseUp(const MouseEvent& e)
{
	if (e.originalComponent == this)
	{

	}
	else
	{
		AutomationKeyUI::Handle* h = dynamic_cast<AutomationKeyUI::Handle*>(e.eventComponent);
		if (h != nullptr)
		{
			int index = h->dimensionIndex;
			AutomationKeyUI* kui = static_cast<AutomationKeyUI*>(h->getParentComponent());
			if (e.mods.isLeftButtonDown())
			{

				Array<UndoableAction*> actions;
				actions.add(kui->item->position->setUndoableValue(kui->posAtMouseDown, kui->item->position->floatValue(), true));
				actions.add(kui->item->values[index]->setUndoableValue(kui->valueAtMouseDown, kui->item->values[index]->floatValue(), true));
				UndoMaster::getInstance()->performActions("Move automation key", actions);

			}
		}
	}
}

bool AutomationUI::keyPressed(const KeyPress& e)
{
	if (e.getKeyCode() == KeyPress::createFromDescription("f").getKeyCode())
	{
		frameViewYRange();
		return true;
	}
	else if (e.getKeyCode() == KeyPress::createFromDescription("h").getKeyCode())
	{
		homeViewYRange();
		return true;
	}

	return BaseManagerUI::keyPressed(e);
	//return false;
}

void AutomationUI::newMessage(const ContainerAsyncEvent& e)
{
	if (e.type == ContainerAsyncEvent::EventType::ControllableFeedbackUpdate)
	{
		if (e.targetControllable == manager->position)
		{
			currentUI = getClosestKeyUIForPos(manager->position->floatValue());
			shouldRepaint = true;
		}
		else if (e.targetControllable == manager->viewValueRange)
		{
			resized();
		}
		else if (manager->values.contains(dynamic_cast<FloatParameter*>(e.targetControllable)))
		{
			shouldRepaint = true;
		}
		else if (e.targetControllable == manager->length)
		{
			if (autoResetViewRangeOnLengthUpdate) setViewRange(0, manager->length->floatValue());
		}
		else if (e.targetControllable != nullptr)
		{
			if (AutomationKey* k = e.targetControllable->getParentAs<AutomationKey>())
			{
				if (e.targetControllable == k->easingType)
				{
					//repaint();
				}
				else if (e.targetControllable == k->position || k->values.contains(dynamic_cast<FloatParameter*>(e.targetControllable)))
				{
					placeKeyUI(getUIForItem(k));
					shouldRepaint = true;

				}
			}
		}
	}
}

void AutomationUI::inspectablesSelectionChanged()
{
	/*
	if (transformer != nullptr)
	{
		removeChildComponent(transformer.get());
		transformer = nullptr;
	}
	*/

	Array<AutomationKeyUI*> uiSelection;
	if (manager->selectionManager->currentInspectables.size() >= 2)
	{

	}

	Array<AutomationKey*> keys = manager->selectionManager->getInspectablesAs<AutomationKey>();
	for (auto& k : keys)
	{
		AutomationKeyUI* kui = getUIForItem(k);
		if (kui == nullptr) return;

		uiSelection.add(kui);
	}

	/*
	if (uiSelection.size() >= 2)
	{
		transformer.reset(new AutomationMultiKeyTransformer(this, uiSelection));
		addAndMakeVisible(transformer.get());
		transformer->grabKeyboardFocus(); // so no specific key has the focus for deleting
	}
	*/
}

void AutomationUI::inspectableDestroyed(Inspectable*)
{
	if (!inspectable.wasObjectDeleted() && manager->selectionManager != nullptr)
	{
		manager->selectionManager->removeSelectionListener(this);
		manager->removeAsyncContainerListener(this);
	}
}

void AutomationUI::focusGained(FocusChangeType cause)
{
	if (autoSwitchMode) setViewMode(EDIT);
}

void AutomationUI::focusLost(FocusChangeType cause)
{
	//DBG("AUI Focus lost " << cause);
	if (autoSwitchMode) setViewMode(VIEW);
}


void AutomationUI::run()
{
	bool firstRun = true;

	while (!threadShouldExit())
	{
		sleep(50); //20ms is plenty enough

		if (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing) continue;
		if (!shouldUpdateImage) continue;

		shouldUpdateImage = false;

		imageLock.enter();

		const int resX = getWidth();
		const int resY = getHeight();

		if (resX == 0 || resY == 0)
		{
			imageLock.exit();
			return;
		}

		if (resX != viewImage.getWidth() || resY != viewImage.getHeight()) viewImage = Image(Image::ARGB, resX, resY, true);
		else viewImage.clear(viewImage.getBounds());


		for (int tx = 0; tx < resX; tx++)
		{
			if (threadShouldExit())
			{
				imageLock.exit();
				return;
			}

			Array<float> values = manager->getValuesForPosition(getPosForX(tx));

			for (int i = 0; i < manager->numDimensions; i++)
			{
				float y = (1 - values[i]) * (getHeight() - 1);
				int ty = (int)y;
				int maxDist = 1;
				for (int j = ty - maxDist; j <= ty + maxDist; j++)
				{
					if (j < 0 || j >= viewImage.getHeight()) continue;
					float alpha = jlimit<float>(0, 1, 1 - (abs(y - j) / maxDist));
					viewImage.setPixelAt(tx, j, Colours::white.withAlpha(alpha));
				}
			}

			//if(ty < viewImage.getHeight()) viewImage.setPixelAt(tx, ty+1, Colours::white.withAlpha(.2f));
			//if(ty > 0) viewImage.setPixelAt(tx, ty-1, Colours::white.withAlpha(.2f));
		}

		imageLock.exit();

		if (firstRun)
		{
			firstRun = false;
			//MessageManagerLock mmLock;
			shouldRepaint = true;
		}
	}

	imageLock.enter();
	viewImage.clear(viewImage.getBounds());
	imageLock.exit();

	//DBG("Exit AutomationUI Thread");
}

void AutomationUI::timerCallback()
{
	if (shouldRepaint)
	{
		repaint();
		shouldRepaint = false;
	}
}
