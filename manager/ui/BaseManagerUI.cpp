#include "JuceHeader.h"


BaseManagerUI::BaseManagerUI(const String& contentName, BaseManager* _manager, bool _useViewport) :
	InspectableContentComponent(_manager),
	baseManager(_manager),
	managerComparator(_manager),
	useViewport(_useViewport),
	defaultLayout(VERTICAL),
	container(this),
	headerSize(24),
	bgColor(BG_COLOR),
	labelHeight(10),
	minHeight(50),
	managerUIName(contentName),
	drawContour(false),
	transparentBG(false),
	resizeOnChildBoundsChanged(true),
	autoFilterHitTestOnItems(false),
	validateHitTestOnNoItem(true),
	showTools(false),
	animateItemOnAdd(true),
	useDedicatedSelector(true),
	selectingItems(false),
	isDraggingOver(false),
	highlightOnDragOver(true),
	fixedItemHeight(true),
	gap(2)
{

	selectionContourColor = LIGHTCONTOUR_COLOR;
	addItemText = "Add Item";

	if (useViewport)
	{
		container.addComponentListener(this);

		viewport.setViewedComponent(&container, false);
		viewport.setScrollBarsShown(true, false);
		viewport.setScrollOnDragMode(Viewport::ScrollOnDragMode::never);
		viewport.setScrollBarThickness(10);
		//viewport.setEnableKeyPressEvents(false);
		this->addAndMakeVisible(viewport);
	}

	//baseManager->addManagerListener(this);
	//baseManager->addAsyncManagerListener(this);

	addItemBT.reset(AssetManager::getInstance()->getAddBT());
	addItemBT->setWantsKeyboardFocus(false);

	addAndMakeVisible(addItemBT.get());
	addItemBT->addListener(this);

	setShowAddButton(baseManager->userCanAddItemsManually);

	acceptedDropTypes.add(baseManager->itemDataType);

	Engine::mainEngine->addAsyncEngineListener(this);

	InspectableSelectionManager::mainSelectionManager->addAsyncSelectionManagerListener(this);
	//must call addExistingItems from child class to get overrides

	setWantsKeyboardFocus(true);
}


BaseManagerUI::~BaseManagerUI()
{
	if (!inspectable.wasObjectDeleted())
	{
		//this->baseManager->removeManagerListener(this);
		//this->baseManager->removeAsyncManagerListener(this);
	}

	Engine::mainEngine->removeAsyncEngineListener(this);
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeAsyncSelectionManagerListener(this);

}

void BaseManagerUI::setDefaultLayout(Layout l)
{
	defaultLayout = l;
	if (useViewport)
	{
		if (defaultLayout == VERTICAL) viewport.setScrollBarsShown(true, false);
		else viewport.setScrollBarsShown(false, true);
	}

}

void BaseManagerUI::addExistingItems(bool resizeAfter)
{

	//add existing items
	for (auto& t : baseManager->baseItems) addItemUI(t, false);
	if (resizeAfter) resized();
}

void BaseManagerUI::setShowAddButton(bool value)
{
	addItemBT->setVisible(value);

	if (value) addAndMakeVisible(addItemBT.get());
	else removeChildComponent(addItemBT.get());
}

void BaseManagerUI::setShowSearchBar(bool value)
{
	if (value)
	{
		searchBar.reset(new TextEditor("SearchBar"));
		searchBar->setSelectAllWhenFocused(true);

		//searchBar->setJustificationType(Justification::topLeft);
		searchBar->setColour(searchBar->backgroundColourId, BG_COLOR.darker(.1f).withAlpha(.7f));
		searchBar->setColour(searchBar->outlineColourId, BG_COLOR.brighter(.1f));
		searchBar->setColour(searchBar->textColourId, TEXT_COLOR.darker(.3f));
		searchBar->setFont(FontOptions(10));
		searchBar->setMultiLine(false);
		searchBar->setColour(CaretComponent::caretColourId, Colours::orange);
		//searchBar->edit(true);
		searchBar->addListener(this);
		searchBar->setTextToShowWhenEmpty("Search...", TEXT_COLOR.withAlpha(.6f));
		addAndMakeVisible(searchBar.get());
	}
	else if (searchBar != nullptr)
	{
		removeChildComponent(searchBar.get());
		searchBar.reset();
	}
}

void BaseManagerUI::setShowTools(bool value)
{
	if (value == showTools) return;

	showTools = value;
	if (value)
	{
		addAndMakeVisible(&toolContainer);
	}
	else
	{
		toolContainer.setVisible(false);
		removeChildComponent(&toolContainer);
	}

	resized();
}

void BaseManagerUI::addButtonTool(Button* c, std::function<void()> clickFunc)
{
	if (tools.contains(c)) return;
	tools.add(c);
	toolContainer.addAndMakeVisible(c);
	if (clickFunc != nullptr) toolFuncMap.set(c, clickFunc);
	c->addListener(this);
}

void BaseManagerUI::addControllableTool(ControllableUI* c)
{
	toolContainer.addAndMakeVisible(c);
	tools.add(c);
}

void BaseManagerUI::mouseDown(const MouseEvent& e)
{
	InspectableContentComponent::mouseDown(e);

	if (e.eventComponent == this)
	{
		if (e.mods.isLeftButtonDown())
		{
			if (!e.mods.isCommandDown() && !e.mods.isAltDown())
			{
				if (useDedicatedSelector)
				{
					Array<Component*> selectables;
					Array<Inspectable*> inspectables;
					addSelectableComponentsAndInspectables(selectables, inspectables);

					InspectableSelector::getInstance()->startSelection(this, selectables, inspectables, nullptr, !e.mods.isShiftDown());
					selectingItems = true;
				}
			}
		}
		else if (e.mods.isRightButtonDown() && e.eventComponent == this)
		{
			if (baseManager->userCanAddItemsManually) showMenuAndAddItem(false, e.getEventRelativeTo(this).getMouseDownPosition());
		}
	}

}

void BaseManagerUI::mouseUp(const MouseEvent& e)
{
	if (e.eventComponent == this)
	{
		if (selectingItems)
		{
			//InspectableSelector::getInstance()->endSelection();
			selectingItems = false;
		}
	}
}

void BaseManagerUI::askSelectToThis(BaseItemMinimalUI* itemUI)
{
	BaseItem* firstItem = InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>();
	int firstIndex = baseManager->baseItems.indexOf(firstItem);
	int itemIndex = baseManager->baseItems.indexOf(itemUI->baseItem);

	if (firstIndex == itemIndex) return;

	if (firstIndex >= 0 && itemIndex >= 0)
	{
		int step = firstIndex < itemIndex ? 1 : -1;

		for (int index = firstIndex; index != itemIndex; index += step)
		{
			baseManager->baseItems[index]->selectThis(true);
		}

	}

	if (itemIndex >= 0) baseManager->baseItems[itemIndex]->selectThis(true);
}


void BaseManagerUI::paint(Graphics& g)
{
	Rectangle<int> r = getLocalBounds();

	if (!transparentBG)
	{
		g.setColour(bgColor);
		g.fillRoundedRectangle(r.toFloat(), 4);
	}

	if (drawContour)
	{
		Colour contourColor = bgColor.brighter(.2f);
		g.setColour(contourColor);
		g.drawRoundedRectangle(r.toFloat(), 4, 2);

		g.setFont(g.getCurrentFont().withHeight(labelHeight));
		float textWidth = (int)TextLayout::getStringWidth(g.getCurrentFont(), managerUIName);
		Rectangle<int> tr = r.removeFromTop(labelHeight + 2).reduced((r.getWidth() - textWidth) / 2, 0).expanded(4, 0);
		g.fillRect(tr);
		Colour textColor = contourColor.withBrightness(contourColor.getBrightness() > .5f ? .1f : .9f).withAlpha(1.f);
		g.setColour(textColor);

		g.drawText(managerUIName, tr, Justification::centred, 1);
	}
	else
	{
		if (!transparentBG)	g.fillAll(bgColor);
	}

	if (isDraggingOver && highlightOnDragOver)
	{
		g.setColour(BLUE_COLOR);

		switch (defaultLayout)
		{
		case HORIZONTAL:
		case VERTICAL:
		{
			if (baseItemsUI.size() > 0)
			{
				BaseItemMinimalUI* bui = dynamic_cast<BaseItemMinimalUI*>(baseItemsUI[currentDropIndex >= 0 ? currentDropIndex : baseItemsUI.size() - 1]);
				if (bui != nullptr)
				{
					Rectangle<int> buiBounds = getLocalArea(bui, bui->getLocalBounds());
					if (defaultLayout == HORIZONTAL)
					{
						int tx = currentDropIndex >= 0 ? buiBounds.getX() - 1 : buiBounds.getRight() + 1;
						g.drawLine(tx, getHeight() * .25f, tx, getHeight() * .75f, 2);
					}
					else if (defaultLayout == VERTICAL)
					{
						int ty = currentDropIndex >= 0 ? buiBounds.getY() - 1 : buiBounds.getBottom() + 1;
						g.drawLine(getWidth() * .25f, ty, getWidth() * .75f, ty, 2);
					}
				}
			}
		}
		break;

		case FREE:
			g.drawRoundedRectangle(r.toFloat(), 4, 2);
			break;
		}
	}


	if (!this->inspectable.wasObjectDeleted() && this->baseManager->baseItems.size() == 0 && noItemText.isNotEmpty())
	{
		g.setColour(Colours::white.withAlpha(.4f));
		g.setFont(FontOptions(jmin(getHeight() - 2, 14)));
		g.drawFittedText(noItemText, getLocalBounds().reduced(5), Justification::centred, 6);
	}
}

void BaseManagerUI::resized()
{
	if (getWidth() == 0 || getHeight() == 0) return;

	//bool resizeOnChange = resizeOnChildBoundsChanged;
	//resizeOnChildBoundsChanged = false; //avoir infinite loop if resize actually resizes inner components

	Rectangle<int> r = getLocalBounds().reduced(2);
	Rectangle<int> hr = setHeaderBounds(r);
	resizedInternalHeader(hr);
	resizedInternalFooter(r);
	resizedInternalContent(r);

	updateItemsVisibility();

	//resizeOnChildBoundsChanged = resizeOnChange;
}

Rectangle<int> BaseManagerUI::setHeaderBounds(Rectangle<int>& r)
{
	return defaultLayout == VERTICAL ? r.removeFromTop(headerSize) : r.removeFromRight(headerSize);
}

void BaseManagerUI::resizedInternalHeader(Rectangle<int>& hr)
{
	if (addItemBT != nullptr && addItemBT->isVisible() && addItemBT->getParentComponent() == this)
	{
		if (defaultLayout == VERTICAL || defaultLayout == FREE) addItemBT->setBounds(hr.removeFromRight(hr.getHeight()).reduced(2));
		else addItemBT->setBounds(hr.removeFromTop(24).removeFromRight(24).reduced(2));
	}

	if (searchBar != nullptr && searchBar->isVisible() && searchBar->getParentComponent() == this)
	{
		if (defaultLayout == VERTICAL || defaultLayout == FREE) searchBar->setBounds(hr.removeFromLeft(150).reduced(2));
		else searchBar->setBounds(hr.removeFromTop(20).reduced(2));
	}

	if (showTools) resizedInternalHeaderTools(hr);
}

void BaseManagerUI::resizedInternalHeaderTools(Rectangle<int>& r)
{
	r.removeFromLeft(4);

	float rSize = 0;
	for (auto& t : this->tools) rSize += jmax(t->getWidth(), r.getHeight());

	r.removeFromLeft((r.getWidth() - rSize) / 2);

	if (r.getWidth() == 0 || r.getHeight() == 0) return;

	toolContainer.setBounds(r.withWidth(rSize));

	Rectangle<int> toolR = toolContainer.getLocalBounds();

	for (auto& t : this->tools)
	{
		Rectangle<int> tr = toolR.removeFromLeft(jmax(t->getWidth(), r.getHeight()));
		tr.reduce(tr.getWidth() == toolR.getHeight() ? 6 : 0, 6);
		t->setBounds(tr);
	}
}

void BaseManagerUI::resizedInternalContent(Rectangle<int>& r)
{
	if (useViewport)
	{
		viewport.setBounds(r);
		if (defaultLayout == VERTICAL) r.removeFromRight(drawContour ? 14 : 12);
		else r.removeFromBottom(drawContour ? 14 : 12);

		r.setY(0);
	}

	placeItems(r);

	if (useViewport || resizeOnChildBoundsChanged)
	{
		if (defaultLayout == VERTICAL)
		{
			float th = 0;
			if (baseItemsUI.size() > 0) th = baseItemsUI[baseItemsUI.size() - 1]->getBottom();
			//if (grabbingItem != nullptr) th = jmax<int>(th + grabbingItem->getHeight(), viewport.getHeight());

			if (useViewport) container.setSize(getWidth(), th);
			else this->setSize(getWidth(), jmax<int>(th + 10, minHeight));
		}
		else if (defaultLayout == HORIZONTAL)
		{
			float tw = 0;
			if (baseItemsUI.size() > 0) tw = baseItemsUI[baseItemsUI.size() - 1]->getRight();
			//if (grabbingItem != nullptr) tw = jmax<int>(tw, viewport.getWidth());
			if (useViewport) container.setSize(tw, getHeight());
			else this->setSize(tw, getHeight());
		}
	}
}

void BaseManagerUI::placeItems(Rectangle<int>& r)
{
	int i = 0;
	for (auto& ui : baseItemsUI)
	{
		if (!checkFilterForItem(ui))
		{
			ui->setVisible(false);
			continue;
		}
		ui->setVisible(true);

		Rectangle<int> tr;
		if (defaultLayout == VERTICAL) tr = r.withHeight(ui->getHeight());
		else tr = r.withWidth(ui->getWidth());

		if (tr != ui->getBounds()) ui->setBounds(tr);

		if (defaultLayout == VERTICAL) r.translate(0, tr.getHeight() + gap);
		else r.translate(tr.getWidth() + gap, 0);

		++i;
	}
}

void BaseManagerUI::resizedInternalFooter(Rectangle<int>& r)
{
}

void BaseManagerUI::alignItems(AlignMode alignMode)
{
	Array<BaseItem*> inspectables = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
	if (inspectables.size() == 0) return;

	float target = (alignMode == BaseManagerUI::AlignMode::CENTER_V || alignMode == BaseManagerUI::AlignMode::CENTER_H) ? 0 : ((alignMode == BaseManagerUI::AlignMode::LEFT || alignMode == BaseManagerUI::AlignMode::TOP) ? INT32_MAX : INT32_MIN);

	Array<BaseItem*> goodInspectables;
	for (auto& i : inspectables)
	{
		if (i == nullptr || !baseManager->baseItems.contains(i)) continue;
		switch (alignMode)
		{
		case BaseManagerUI::AlignMode::LEFT: target = jmin(i->viewUIPosition->x, target); break;
		case BaseManagerUI::AlignMode::RIGHT: target = jmax(i->viewUIPosition->x + i->viewUISize->x, target); break;
		case BaseManagerUI::AlignMode::CENTER_H: target += i->viewUIPosition->x + i->viewUISize->x / 2; break;
		case BaseManagerUI::AlignMode::TOP: target = jmin(target, i->viewUIPosition->y); break;
		case BaseManagerUI::AlignMode::BOTTOM: target = jmax(target, i->viewUIPosition->y + i->viewUISize->y); break;
		case BaseManagerUI::AlignMode::CENTER_V: target += i->viewUIPosition->y + i->viewUISize->y / 2; break;
		}
		goodInspectables.add(i);
	}

	if (goodInspectables.size() == 0) return;

	if (alignMode == CENTER_H || alignMode == CENTER_V) target /= goodInspectables.size();

	Array<UndoableAction*> actions;
	for (auto& i : goodInspectables)
	{
		Point<float> targetPoint;
		switch (alignMode)
		{
		case BaseManagerUI::AlignMode::LEFT: targetPoint = Point<float>(target, i->viewUIPosition->y); break;
		case BaseManagerUI::AlignMode::RIGHT: targetPoint = Point<float>(target - i->viewUISize->x, i->viewUIPosition->y); break;
		case BaseManagerUI::AlignMode::CENTER_H: targetPoint = Point<float>(target - i->viewUISize->x / 2, i->viewUIPosition->y);  break;
		case BaseManagerUI::AlignMode::TOP: targetPoint = Point<float>(i->viewUIPosition->x, target); break;
		case BaseManagerUI::AlignMode::BOTTOM:targetPoint = Point<float>(i->viewUIPosition->x, target - i->viewUISize->y); break;
		case BaseManagerUI::AlignMode::CENTER_V:targetPoint = Point<float>(i->viewUIPosition->x, target - i->viewUISize->y / 2); break;
		}
		actions.addArray(i->viewUIPosition->setUndoablePoint(targetPoint, true));
	}

	UndoMaster::getInstance()->performActions("Align " + String(goodInspectables.size()) + " items", actions);

}

void BaseManagerUI::distributeItems(bool isVertical)
{
	Array<BaseItem*> inspectables = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
	if (inspectables.size() < 3) return;


	Array<BaseItem*> goodInspectables;
	for (auto& i : inspectables)
	{
		if (i == nullptr || !baseManager->baseItems.contains(i)) continue;

		goodInspectables.add(i);
	}

	if (goodInspectables.size() < 3) return;


	PositionComparator comp(isVertical);
	goodInspectables.sort(comp);

	Point<float> center0 = goodInspectables[0]->viewUIPosition->getPoint() + goodInspectables[0]->viewUISize->getPoint() / 2;
	Point<float> center1 = goodInspectables[goodInspectables.size() - 1]->viewUIPosition->getPoint() + goodInspectables[goodInspectables.size() - 1]->viewUISize->getPoint() / 2;
	float tMin = isVertical ? center0.y : center0.x;
	float tMax = isVertical ? center1.y : center1.x;

	Array<UndoableAction*> actions;
	for (int i = 0; i < goodInspectables.size(); i++)
	{
		float rel = i * 1.0f / (goodInspectables.size() - 1);
		float target = jmap(rel, tMin, tMax);
		BaseItem* ti = goodInspectables[i];
		Point<float> targetPoint(isVertical ? ti->viewUIPosition->x : target - ti->viewUISize->x / 2, isVertical ? target - ti->viewUISize->y / 2 : ti->viewUIPosition->y);
		actions.addArray(ti->viewUIPosition->setUndoablePoint(targetPoint, true));
	}

	UndoMaster::getInstance()->performActions("Distribute " + String(goodInspectables.size()) + " items", actions);
}


void BaseManagerUI::updateItemsVisibility()
{
	for (auto& bui : baseItemsUI) updateItemVisibilityManagerInternal(bui);

}

void BaseManagerUI::updateItemVisibilityManagerInternal(BaseItemMinimalUI* bui)
{
	updateBaseItemVisibility(bui);
}

void BaseManagerUI::updateBaseItemVisibility(BaseItemMinimalUI* bui)
{
	if (!checkFilterForItem(bui)) return;

	Rectangle<int> vr = this->getLocalArea(bui, bui->getLocalBounds());
	if (defaultLayout == VERTICAL)
	{
		if (viewport.getHeight() > 0 && (vr.getY() > viewport.getBounds().getBottom() || vr.getBottom() < viewport.getY())) bui->setVisible(false);
		else bui->setVisible(true);
	}
	else
	{
		bui->setVisible(true);
	}
}

bool BaseManagerUI::hasFiltering()
{
	return searchBar != nullptr && searchBar->getText().isNotEmpty();
}

Array<BaseItemMinimalUI*> BaseManagerUI::getFilteredItems()
{
	if (!this->hasFiltering()) return Array<BaseItemMinimalUI*>(this->baseItemsUI.getRawDataPointer(), this->baseItemsUI.size());

	Array<BaseItemMinimalUI*> result;
	for (auto& ui : this->baseItemsUI) if (checkFilterForItem(ui)) result.add(ui);
	return result;
}

bool BaseManagerUI::checkFilterForItem(BaseItemMinimalUI* ui)
{
	if (!this->hasFiltering() || searchBar == nullptr) return true;
	return ui->baseItem->niceName.toLowerCase().contains(searchBar->getText().toLowerCase());
}

void BaseManagerUI::childBoundsChanged(Component* c)
{
	if (resizeOnChildBoundsChanged && c != &viewport) resized();
}

bool BaseManagerUI::hitTest(int x, int y)
{
	if (!autoFilterHitTestOnItems) return InspectableContentComponent::hitTest(x, y);
	if (baseItemsUI.size() == 0) return validateHitTestOnNoItem;

	Point<int> p(x, y);
	for (auto& i : baseItemsUI)
	{
		if (i->getBounds().contains(p))
		{
			Point<int> localP = i->getLocalPoint(this, p);
			if (i->hitTest(localP.x, localP.y)) return true;
		}
	}

	return false;
}

void BaseManagerUI::componentMovedOrResized(Component& c, bool wasMoved, bool wasResized)
{
	if (&c == &container && useViewport && !itemAnimator.isAnimating())
	{
		resized();
	}
}

void BaseManagerUI::showMenuAndAddItem(bool isFromAddButton, Point<int> mouseDownPos)
{
	showMenuAndAddItem(isFromAddButton, mouseDownPos, nullptr);
}

void BaseManagerUI::showMenuAndAddItem(bool isFromAddButton, Point<int> mouseDownPos, std::function<void(BaseItem*)> callback)
{
	PopupMenu p;
	p.addItem(1, addItemText);

	addMenuExtraItems(p, 2);

	p.showMenuAsync(PopupMenu::Options(), [this, isFromAddButton, mouseDownPos, callback](int result)
		{
			if (result == 0) return;

			switch (result)
			{
			case 1:
				this->addItemFromMenu(isFromAddButton, mouseDownPos);
				break;

			default:
				this->handleMenuExtraItemsResult(result, 2);
				break;
			}

			//if (callback != nullptr) callback(item);
		}
	);
}

void BaseManagerUI::addItemFromMenu(bool fromAddButton, Point<int> pos)
{
	addBaseItemFromMenu(nullptr, fromAddButton, pos);
}

void BaseManagerUI::addBaseItemFromMenu(BaseItem* item, bool, Point<int>)
{
	baseManager->BaseManager::addItem(item);
}

BaseItemMinimalUI* BaseManagerUI::addItemUI(BaseItem* item, bool animate, bool resizeAndRepaint)
{
	if (item == nullptr) return nullptr;
	BaseItemMinimalUI* tui = createBaseUIForItem(item);
	jassert(tui != nullptr);

	if (useViewport) container.addAndMakeVisible(tui);
	else addAndMakeVisible(tui);

	int index = baseManager->baseItems.indexOf(item);
	baseItemsUI.insert(index, tui);

	tui->addItemMinimalUIListener(this);

	BaseItemUI* biui = dynamic_cast<BaseItemUI*>(tui);
	if (biui != nullptr) biui->addItemUIListener(this);

	addItemUIManagerInternal(tui);

	if (animate && !Engine::mainEngine->isLoadingFile)
	{
		Rectangle<int> tb = tui->getBounds();
		tui->setSize(10, 10);
		itemAnimator.animateComponent(tui, tb, 1, 200, false, 1, 0);
	}
	else
	{
		if (biui != nullptr && biui->baseItem->miniMode->boolValue()) biui->updateMiniModeUI();
		//DBG("resized");  
		//resized();
	}

	notifyItemUIAdded(tui);

	repaint();

	return tui;
}

BaseItemMinimalUI* BaseManagerUI::createBaseUIForItem(BaseItem* item)
{
	return new BaseItemMinimalUI(item);
}

void BaseManagerUI::removeItemUI(BaseItem* item, bool resizeAndRepaint)
{
	{
		if (!MessageManager::getInstance()->isThisTheMessageThread())
		{
			MessageManager::getInstance()->callAsync([this, item, resizeAndRepaint]()
				{
					removeItemUI(item, resizeAndRepaint);
				});
			return;
		}

		//MessageManagerLock mmLock; //Ensure this method can be called from another thread than the UI one

		BaseItemMinimalUI* tui = getBaseUIForItem(item, false);
		if (tui == nullptr) return;


		if (useViewport) container.removeChildComponent(tui);
		else removeChildComponent(tui);

		tui->removeItemMinimalUIListener(this);

		BaseItemUI* biui = dynamic_cast<BaseItemUI*>(tui);
		if (biui != nullptr) biui->removeItemUIListener(this);

		baseItemsUI.removeObject(tui, false);
		removeItemUIManagerInternal(tui);

		notifyItemUIRemoved(tui);

		delete tui;

		if (resizeAndRepaint)
		{
			resized();
			repaint();
		}
	}
}

BaseItemMinimalUI* BaseManagerUI::getBaseUIForItem(BaseItem* item, bool directIndexAccess)
{
	if (directIndexAccess) return baseItemsUI[baseManager->baseItems.indexOf(item)];

	for (auto& ui : baseItemsUI) if (ui->baseItem == item) return ui; //brute search, not needed if ui/items are synchronized
	return nullptr;
}

int BaseManagerUI::getContentHeight()
{
	return container.getHeight() + 20;
}


void BaseManagerUI::itemAddedAsync(BaseItem* item)
{
	addItemUI(item, animateItemOnAdd);
	if (!animateItemOnAdd) resized();
}

void BaseManagerUI::itemsAddedAsync(Array<BaseItem*> items)
{
	for (auto& i : items) addItemUI(i, false, false);

	resized();
	repaint();
}

void BaseManagerUI::groupAddedAsync(BaseItem* item)
{
	addItemUI(item, animateItemOnAdd);
	if (!animateItemOnAdd) resized();
}

void BaseManagerUI::groupsAddedAsync(Array<BaseItem*> items)
{
	for (auto& i : items) addItemUI(i, false, false);

	resized();
	repaint();
}

void BaseManagerUI::itemsReorderedAsync()
{
	baseItemsUI.sort(managerComparator);
	resized();
}

void BaseManagerUI::newMessage(const InspectableSelectionManager::SelectionEvent& e)
{
	if (e.type == e.SELECTION_CHANGED)
	{
		if (useViewport)
		{
			//			if (BaseItem* item = InspectableSelectionManager::activeSelectionManager->getInspectableAs<T>())
			//			{
			//				if (BaseItemMinimalUI* ui = getUIForItem(item))
			//				{
			//					if (defaultLayout == HORIZONTAL) viewport.setViewPosition(ui->getX() + this->getWidth() / 2, 0);
			//					else if (defaultLayout == VERTICAL) viewport.setViewPosition(0, ui->getY() + this->getHeight() / 2);
			//				}
			//			}
		}
	}
}

bool BaseManagerUI::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (acceptedDropTypes.contains(dragSourceDetails.description.getProperty("dataType", "").toString())) return true;

	BaseItemMinimalUI* itemUI = dynamic_cast<BaseItemMinimalUI*>(dragSourceDetails.sourceComponent.get());
	if (baseItemsUI.contains(itemUI)) return true;

	return false;
}

void BaseManagerUI::itemDragEnter(const SourceDetails&)
{
	isDraggingOver = true;
	repaint();
}

void BaseManagerUI::itemDragMove(const SourceDetails& dragSourceDetails)
{
	if (defaultLayout == HORIZONTAL || defaultLayout == VERTICAL)
	{
		currentDropIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
		repaint();
	}
}


void BaseManagerUI::itemDragExit(const SourceDetails&)
{
	isDraggingOver = false;
	repaint();
}

void BaseManagerUI::itemDropped(const SourceDetails& dragSourceDetails)
{

	if (defaultLayout == HORIZONTAL || defaultLayout == VERTICAL)
	{
		if (BaseItemMinimalUI* bui = dynamic_cast<BaseItemMinimalUI*>(dragSourceDetails.sourceComponent.get()))
		{
			if (BaseItem* item = bui->baseItem)
			{
				int droppingIndex = getDropIndexForPosition(dragSourceDetails.localPosition);
				if (baseItemsUI.contains(bui))
				{
					if (baseItemsUI.indexOf(bui) < droppingIndex) droppingIndex--;
					if (droppingIndex == -1) droppingIndex = baseItemsUI.size() - 1;
					this->baseManager->setItemIndex(item, droppingIndex);
				}
				else
				{
					var data = item->getJSONData();
					if (droppingIndex != -1) data.getDynamicObject()->setProperty("index", droppingIndex);

					if (BaseItem* newItem = this->baseManager->createItemFromData(data))
					{
						Array<UndoableAction*> actions;
						actions.add(this->baseManager->getAddBaseItemUndoableAction(newItem, data));
						if (BaseManager* sourceManager = dynamic_cast<BaseManager*>(item->parentContainer.get()))
						{
							actions.addArray(sourceManager->getRemoveBaseItemUndoableAction(item));
						}
						UndoMaster::getInstance()->performActions("Move " + item->niceName, actions);
					}
				}
			}
		}
	}

	this->isDraggingOver = false;
	repaint();
}

void BaseManagerUI::addSelectableComponentsAndInspectables(Array<Component*>& selectables, Array<Inspectable*>& inspectables)
{
	for (auto& i : baseItemsUI)
	{
		if (i->isVisible())
		{
			selectables.add(getSelectableComponentForBaseItemUI(i));
			inspectables.add(i->inspectable);
		}
	}

}

Component* BaseManagerUI::getSelectableComponentForBaseItemUI(BaseItemMinimalUI* itemUI)
{
	return itemUI;
}

int BaseManagerUI::getDropIndexForPosition(Point<int> localPosition)
{
	for (int i = 0; i < baseItemsUI.size(); ++i)
	{
		BaseItemMinimalUI* iui = dynamic_cast<BaseItemMinimalUI*>(baseItemsUI[i]);
		Point<int> p = getLocalArea(iui, iui->getLocalBounds()).getCentre();

		if (defaultLayout == HORIZONTAL && localPosition.x < p.x) return i;
		else if (defaultLayout == VERTICAL && localPosition.y < p.y) return i;
	}

	return -1;
}


void BaseManagerUI::buttonClicked(Button* b)
{
	if (b == addItemBT.get())
	{
		showMenuAndAddItem(true, Point<int>());
	}
	else if (this->toolFuncMap.contains(b))
	{
		this->toolFuncMap[b]();
	}
}

void BaseManagerUI::textEditorTextChanged(TextEditor& e)
{
	if (&e == searchBar.get())
	{
		resized();
	}
}

void BaseManagerUI::textEditorReturnKeyPressed(TextEditor& e)
{
	if (&e == searchBar.get())
	{
		searchBar->unfocusAllComponents();
	}
}

void BaseManagerUI::inspectableDestroyed(Inspectable*)
{
	//to be overriden in templated class
}

void BaseManagerUI::newMessage(const Engine::EngineEvent& e)
{
	resized();
}

void BaseManagerUIItemContainer::childBoundsChanged(juce::Component* c) {
	baseManagerUI->childBoundsChanged(c);
}
