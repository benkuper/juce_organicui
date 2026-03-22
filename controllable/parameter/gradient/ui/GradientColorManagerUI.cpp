/*
  ==============================================================================

	GradientColorManagerUI.cpp
	Created: 11 Apr 2017 11:40:47am
	Author:  Ben

  ==============================================================================
*/


GradientColorManagerUI::GradientColorManagerUI(GradientColorManager* manager) :
	BaseManagerUI(manager->niceName, manager, false),
	UITimerTarget(ORGANICUI_SLOW_TIMER, "GradientColorManagerUI", true),
	Thread("Colors " + String(manager->niceName)),
	shouldUpdateImage(true),
	autoResetViewRangeOnLengthUpdate(false),
	miniMode(false)
{
	setShowAddButton(false);
	animateItemOnAdd = false;

	transparentBG = true;
	setViewRange(0, manager->length->floatValue());

	resizeOnChildBoundsChanged = false;

	manager->addAsyncCoalescedContainerListener(this);
	addExistingItems();

	startThread();
}

GradientColorManagerUI::~GradientColorManagerUI()
{
	if(!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
	stopThread(1000);
	//stopThread(100);
}

void GradientColorManagerUI::setViewRange(float start, float end)
{

	viewStartPos = start;
	viewEndPos = end;

	shouldUpdateImage = true;
	resized();
}

void GradientColorManagerUI::setMiniMode(bool value)
{
	if (miniMode == value) return;
	miniMode = value;

}

void GradientColorManagerUI::paint(Graphics& g)
{
	BaseManagerUI::paint(g);

	juce::Rectangle<int> r = getLocalBounds();

	if (!miniMode)
	{
		g.setColour(manager->currentColor->getColor());
		g.fillRect(r.removeFromBottom(16).reduced(2, 7));
	}

	g.fillCheckerBoard(r.toFloat(), 12, 12, Colours::white, Colours::white.darker(.2f));
	imageLock.enter();
	g.setColour(Colours::white);
	g.drawImage(viewImage, r.toFloat());
	imageLock.exit();

	/*
	g.setColour(manager->currentColor->getColor());
	g.fillRect(r.removeFromBottom(16).reduced(2,7));z

	manager->gradient.point1.setX(getXForPos(0));
	manager->gradient.point2.setX(getXForPos(manager->length->floatValue()));

	g.fillCheckerBoard(r.toFloat(), 12, 12, Colours::white, Colours::white.darker(.2f));
	if (!manager->items.isEmpty())
	{
		g.setGradientFill(manager->gradient);
		g.fillRect(r);
	}
	*/
}

void GradientColorManagerUI::paintOverChildren(Graphics& g)
{
	validatePaint();
}

void GradientColorManagerUI::resized()
{

	if (getParentComponent() == nullptr) return;
	if (getWidth() == 0 || getHeight() == 0) return;
	if (itemsUI.size() == 0) return;

	shouldUpdateImage = true;

	for (auto& tui : itemsUI)
	{
		tui->setVisible(!miniMode && isInView(tui));
		if (tui->isVisible()) placeItemUI(tui);
	}
}

void GradientColorManagerUI::updateItemsVisibility()
{
	BaseManagerUI::updateItemsVisibility();
}

void GradientColorManagerUI::addItemUIInternal(GradientColorUI* item)
{
	item->addMouseListener(this, true);
	placeItemUI(item);
}

void GradientColorManagerUI::removeItemUIInternal(GradientColorUI* item)
{
	item->removeMouseListener(this);
}

void GradientColorManagerUI::mouseDown(const MouseEvent& e)
{
	BaseManagerUI::mouseDown(e);

	if (GradientColorUI* handle = dynamic_cast<GradientColorUI*>(e.eventComponent))
	{
		snapTimes.clear();
		if (getSnapTimesFunc != nullptr) getSnapTimesFunc(&snapTimes);
	}
}

void GradientColorManagerUI::mouseDoubleClick(const MouseEvent& e)
{
	if (e.originalComponent == this && !miniMode)
	{
		float pos = getPosForX(e.getMouseDownX());
		manager->addColorAt(pos, manager->getColorForPosition(pos));
	}
}


void GradientColorManagerUI::mouseDrag(const MouseEvent& e)
{
	if (miniMode) return;

	if (e.originalComponent == this)
	{
		BaseManagerUI::mouseDrag(e);
	}
	else
	{
		GradientColorUI* tui = dynamic_cast<GradientColorUI*>(e.eventComponent);
		if (tui == nullptr) tui = dynamic_cast<GradientColorUI*>(e.eventComponent->getParentComponent());

		if (tui != nullptr)
		{
			if (e.mods.isLeftButtonDown())
			{
				float diffTime = getPosForX(e.getOffsetFromDragStart().x, false);

				if (e.mods.isShiftDown())
				{
					float targetTime = tui->item->movePositionReference.x + diffTime;
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

					diffTime = targetTime - tui->item->movePositionReference.x;
				}

				Point<float> p(diffTime, 0.f);
				tui->item->BaseItem::movePosition(p, true);
			}
		}
	}
}

void GradientColorManagerUI::placeItemUI(GradientColorUI* tui)
{
	if (tui == nullptr) return;
	int tx = getXForPos(tui->item->position->floatValue());
	tui->setBounds(tx - 6, getHeight() - 18, 12, 16);
}

int GradientColorManagerUI::getXForPos(float time)
{
	if (viewStartPos == viewEndPos) return 0;
	return (int)jmap<float>(time, viewStartPos, viewEndPos, 0, (float)getWidth());
}

float GradientColorManagerUI::getPosForX(int tx, bool offsetStart)
{
	float viewRange = viewEndPos - viewStartPos;
	float mapStart = offsetStart ? viewStartPos : 0;
	return jmap<float>((float)tx, 0, (float)getWidth(), mapStart, mapStart + viewRange);
}

bool GradientColorManagerUI::isInView(GradientColorUI* kui)
{
	return kui->item->position->floatValue() >= viewStartPos && kui->item->position->floatValue() <= viewEndPos;
}

void GradientColorManagerUI::newMessage(const ContainerAsyncEvent& e)
{
	if (e.targetControllable.wasObjectDeleted()) return;
	if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
	{
		//if (ControllableUtil::findParentAs<GradientColorManager>(e.targetControllable) != nullptr)
		//{

		if (e.targetControllable == manager->length)
		{
			if (autoResetViewRangeOnLengthUpdate) setViewRange(0, manager->length->floatValue());
			shouldUpdateImage = true;
			shouldRepaint = true;
		}
		else if (e.targetControllable == manager->currentColor)
		{
			shouldRepaint = true;
		}
		else
		{
			GradientColor* gc = e.targetControllable->getParentAs<GradientColor>();
			if (gc != nullptr && (e.targetControllable == gc->position || e.targetControllable == gc->itemColor || e.targetControllable == gc->interpolation))
			{
				shouldUpdateImage = true;
				if (e.targetControllable == gc->position) placeItemUI(getUIForItem(gc, false));
			}
		}
	}
}

void GradientColorManagerUI::run()
{
	while (!threadShouldExit())
	{
		wait(50); //20ms is plenty enough

		if (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing) continue;
		if (!shouldUpdateImage) continue;


		imageLock.enter();

		const int resX = getWidth();
		const int resY = 1;

		if (resX == 0 || resY == 0)
		{
			imageLock.exit();
			return;
		}

		if (resX != viewImage.getWidth() || resY != viewImage.getHeight()) viewImage = Image(Image::ARGB, resX, resY, true);
		else viewImage.clear(viewImage.getBounds());


		if (threadShouldExit())
		{
			imageLock.exit();
			return;
		}

		//manager->gradientLock.enter();
		for (int tx = 0; tx < resX; tx++)
		{
			if (threadShouldExit())
			{
				//manager->gradientLock.exit();
				imageLock.exit();
				return;
			}

			Colour col = manager->getColorForPosition(getPosForX(tx));
			viewImage.setPixelAt(tx, 0, col);
		}

		//manager->gradientLock.exit();
		imageLock.exit();

		shouldUpdateImage = false;
		shouldRepaint = true;
	}

	imageLock.enter();
	viewImage.clear(viewImage.getBounds());
	imageLock.exit();

}

void GradientColorManagerUI::handlePaintTimerInternal()
{
	repaint();
}
