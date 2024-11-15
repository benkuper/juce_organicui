/*
  ==============================================================================

	BaseManagerViewUI.h
	Created: 23 Apr 2017 2:48:02pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

template<class M, class T, class U>
class BaseManagerViewUI :
	public BaseManagerUI<M, T, U>
{
public:
	BaseManagerViewUI(const juce::String& contentName, M* _manager);
	virtual ~BaseManagerViewUI();

	bool canNavigate;

	//
	bool centerUIAroundPosition;
	bool updatePositionOnDragMove;
	bool enableSnapping;
	const int snappingThreshold = 10;
	const int snappingSpacing = 10;
	juce::Line<int> snapLineX;
	juce::Line<int> snapLineY;
	juce::Point<float> targetSnapViewPosition;

	//Checkers
	bool useCheckersAsUnits;
	int checkerSize = 128;

	//Zoom
	bool canZoom;
	bool zoomAffectsItemSize;
	float minZoom;
	float maxZoom;

	double timeSinceLastWheel;


	juce::Point<int> initViewOffset;

	std::unique_ptr<BaseManagerViewMiniPane<M, T, U>> viewPane;

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseDrag(const juce::MouseEvent& e) override;
	virtual void mouseUp(const juce::MouseEvent& e) override;
	void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d) override;
	void mouseMagnify(const juce::MouseEvent& e, float scaleFactor) override;
	virtual bool keyPressed(const juce::KeyPress& e) override;

	virtual void paint(juce::Graphics& g) override;
	virtual void paintBackground(juce::Graphics& g);
	virtual void paintOverChildren(juce::Graphics& g) override;

	virtual void resized() override;

	virtual void updateViewUIPosition(U* se);
	virtual void updateComponentViewPosition(juce::Component* c, juce::Point<float> position, const juce::AffineTransform& af);
	virtual juce::AffineTransform getUITransform(U* se);

	virtual void updateItemsVisibility() override;
	virtual bool checkItemShouldBeVisible(U* se) { return true; } //to be overriden

	virtual void parentHierarchyChanged() override;

	virtual void addItemFromMenu(bool isFromAddButton, juce::Point<int> mouseDownPos) override;
	virtual void addItemFromMenu(T* item, bool isFromAddButton, juce::Point<int> mouseDownPos) override;

	juce::Point<int> getSize();
	juce::Point<float> getViewMousePosition();
	juce::Point<float> getViewPos(const juce::Point<int>& originalPos);
	juce::Point<float> getViewOffset(const juce::Point<int>& offsetInView);
	juce::Rectangle<float> getViewBounds(const juce::Rectangle<int>& originalBounds);
	juce::Point<int> getViewCenter();
	juce::Point<int> getPosInView(const juce::Point<float>& viewPos);
	juce::Rectangle<int> getBoundsInView(const juce::Rectangle<float>& r);
	juce::Point<float> getItemsCenter();

	virtual void homeView();
	virtual void frameView(U* item = nullptr);

	virtual void setViewZoom(float newZoom);
	virtual void setShowPane(bool val);
	virtual BaseManagerViewMiniPane<M, T, U>* createViewPane();

	virtual void addItemUIInternal(U* se) override;

	void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
	void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;

	virtual juce::Point<float> getPositionFromDrag(juce::Component* c, const juce::DragAndDropTarget::SourceDetails& dragSourceDetails);

	virtual void itemUIMiniModeChanged(BaseItemUI<T>* itemUI) override;

	virtual void itemUIViewPositionChanged(BaseItemMinimalUI<T>* itemUI) override;
	virtual void askForSyncPosAndSize(BaseItemMinimalUI<T>* itemUI) override;

	virtual void itemUIResizeDrag(BaseItemMinimalUI<T>* itemUI, const juce::Point<int>& dragOffset) override;
	virtual void itemUIResizeEnd(BaseItemMinimalUI<T>* itemUI) override;

	//snapping
	class SnapResult
	{
	public:
		SnapResult(BaseItemMinimalUI<T>* targetUI, float pos) : targetUI(targetUI), pos(pos) {}
		BaseItemMinimalUI<T>* targetUI;
		float pos;
	};

	virtual SnapResult getClosestSnapUI(juce::Point<float> pos, bool isX, float spacingBefore, float spacingAfter);

};

template<class M, class T, class U>
BaseManagerViewUI<M, T, U>::BaseManagerViewUI(const juce::String& contentName, M* _manager) :
	BaseManagerUI<M, T, U>(contentName, _manager, false),
	canNavigate(true),
	centerUIAroundPosition(false),
	updatePositionOnDragMove(false),
	enableSnapping(false),
	useCheckersAsUnits(false),
	canZoom(true),
	zoomAffectsItemSize(true),
	minZoom(.1f),
	maxZoom(1),
	timeSinceLastWheel(0)
{
	this->defaultLayout = this->FREE;

	this->resizeOnChildBoundsChanged = false;
	this->bgColor = BG_COLOR.darker(.3f);

	this->setWantsKeyboardFocus(true);

	this->headerSize = 28;

	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::align_left_png, OrganicUIBinaryData::align_left_pngSize)), [this]() { this->alignItems(BaseManagerUI<M, T, U>::AlignMode::LEFT); });

	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::align_center_h_png, OrganicUIBinaryData::align_center_h_pngSize)), [this]() { this->alignItems(BaseManagerUI<M, T, U>::AlignMode::CENTER_H); });
	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::align_right_png, OrganicUIBinaryData::align_right_pngSize)), [this]() { this->alignItems(BaseManagerUI<M, T, U>::AlignMode::RIGHT); });
	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::align_top_png, OrganicUIBinaryData::align_top_pngSize)), [this]() { this->alignItems(BaseManagerUI<M, T, U>::AlignMode::TOP); });
	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::align_center_v_png, OrganicUIBinaryData::align_center_v_pngSize)), [this]() { this->alignItems(BaseManagerUI<M, T, U>::AlignMode::CENTER_V); });
	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::align_bottom_png, OrganicUIBinaryData::align_bottom_pngSize)), [this]() { this->alignItems(BaseManagerUI<M, T, U>::AlignMode::BOTTOM); });
	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::distribute_h_png, OrganicUIBinaryData::distribute_h_pngSize)), [this]() { this->distributeItems(false); });
	this->addButtonTool(AssetManager::getInstance()->getSetupBTImage(juce::ImageCache::getFromMemory(OrganicUIBinaryData::distribute_v_png, OrganicUIBinaryData::distribute_v_pngSize)), [this]() { this->distributeItems(true); });

	if (this->manager->snapGridMode != nullptr)
	{
		this->addControllableTool(this->manager->snapGridMode->createToggle(juce::ImageCache::getFromMemory(OrganicUIBinaryData::snap_grid_png, OrganicUIBinaryData::snap_grid_pngSize)));
		this->addControllableTool(this->manager->showSnapGrid->createToggle(juce::ImageCache::getFromMemory(OrganicUIBinaryData::show_grid_png, OrganicUIBinaryData::show_grid_pngSize)));
	}

	for (auto& t : this->tools)  t->setSize(16, 16);

	if (this->manager->snapGridMode != nullptr)
	{
		ControllableUI* sizeUI = this->manager->snapGridSize->createDefaultUI();
		sizeUI->setSize(60, 16);
		this->addControllableTool(sizeUI);
	}
}


template<class M, class T, class U>
BaseManagerViewUI<M, T, U>::~BaseManagerViewUI()
{
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseDown(const juce::MouseEvent& e)
{
	BaseManagerUI<M, T, U>::mouseDown(e);
	if (canNavigate && ((e.mods.isLeftButtonDown() && e.mods.isAltDown()) || e.mods.isMiddleButtonDown()))
	{
		this->setMouseCursor(juce::MouseCursor::UpDownLeftRightResizeCursor);
		this->updateMouseCursor();
		initViewOffset = juce::Point<int>(this->manager->viewOffset.x, this->manager->viewOffset.y);
	}
	else if (juce::ResizableCornerComponent* r = dynamic_cast<juce::ResizableCornerComponent*>(e.eventComponent))
	{
		BaseItemUI<T>* ui = (BaseItemUI<T>*)r->getParentComponent();
		ui->item->setSizeReference(true);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseDrag(const juce::MouseEvent& e)
{
	BaseManagerUI<M, T, U>::mouseDrag(e);

	if (canNavigate && ((e.mods.isLeftButtonDown() && e.mods.isAltDown()) || e.mods.isMiddleButtonDown()))
	{
		this->manager->viewOffset = initViewOffset + e.getOffsetFromDragStart();
		updateItemsVisibility();

		this->resized();
		this->repaint();

		if (viewPane != nullptr)
		{
			viewPane->updateContent();
			viewPane->toFront(false);
		}

	}
	else if (juce::ResizableCornerComponent* r = dynamic_cast<juce::ResizableCornerComponent*>(e.eventComponent))
	{
		BaseItemUI<T>* ui = (BaseItemUI<T>*)r->getParentComponent();
		juce::Point<float> sizeOffset = ui->item->getItemSize() - ui->item->sizeReference;
		ui->item->resizeItem(sizeOffset, true);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseUp(const juce::MouseEvent& e)
{
	BaseManagerUI<M, T, U>::mouseUp(e);

	snapLineX = juce::Line<int>();
	snapLineY = juce::Line<int>();

	this->setMouseCursor(juce::MouseCursor::NormalCursor);
	this->updateMouseCursor();

	if (juce::ResizableCornerComponent* r = dynamic_cast<juce::ResizableCornerComponent*>(e.eventComponent))
	{
		BaseItemUI<T>* ui = (BaseItemUI<T>*)r->getParentComponent();
		juce::Point<float> sizeOffset = ui->item->getItemSize() - ui->item->sizeReference;
		ui->item->addResizeToUndoManager(true);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& d)
{

	if (e.mods.isShiftDown())
	{
		if (canZoom)
		{
			juce::Point<float> curMousePos = getViewMousePosition();

			setViewZoom(this->manager->viewZoom + d.deltaY);

			juce::Point<float> newMousePos = getViewMousePosition();
			this->manager->viewOffset += ((newMousePos - curMousePos) * this->manager->viewZoom * (useCheckersAsUnits ? checkerSize : 1)).toInt();
			updateItemsVisibility();
			this->resized();
		}
	}
	else
	{

		double curTime = juce::Time::getApproximateMillisecondCounter() / 1000.0;

		if (e.originalComponent != this)
		{
			if (curTime - timeSinceLastWheel > 1)
			{
				BaseItemMinimalUI<T>* bui = e.originalComponent->findParentComponentOfClass<BaseItemMinimalUI<T>>();
				if (bui != nullptr && bui->isUsingMouseWheel()) return;
			}
		}

		timeSinceLastWheel = curTime;

		float sensitivity = 500;
		this->manager->viewOffset += juce::Point<int>(d.deltaX * sensitivity, d.deltaY * sensitivity);
		updateItemsVisibility();
		this->resized();
		this->repaint();
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseMagnify(const juce::MouseEvent& e, float scaleFactor)
{
	if (canZoom)
	{
		juce::Point<float> curMousePos = getViewMousePosition();

		setViewZoom(this->manager->viewZoom + scaleFactor);

		juce::Point<float> newMousePos = getViewMousePosition();
		this->manager->viewOffset += ((newMousePos - curMousePos) * this->manager->viewZoom).toInt();
		this->resized();
	}
}

template<class M, class T, class U>
bool BaseManagerViewUI<M, T, U>::keyPressed(const juce::KeyPress& e)
{
	if (BaseManagerUI<M, T, U>::keyPressed(e)) return true;

	if (e.getTextDescription() == juce::KeyPress::createFromDescription("f").getTextDescription())
	{
		frameView();
		return true;
	}
	else if (e.getTextDescription() == juce::KeyPress::createFromDescription("h").getTextDescription())
	{
		homeView();
		return true;
	}

	return false;
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::paint(juce::Graphics& g)
{
	if (this->inspectable.wasObjectDeleted()) return;

	if (!this->transparentBG) paintBackground(g);
	if (this->manager->items.size() == 0 && this->noItemText.isNotEmpty())
	{
		g.setColour(juce::Colours::white.withAlpha(.4f));
		g.setFont(16);
		g.drawFittedText(this->noItemText, this->getLocalBounds(), juce::Justification::centred, 6);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::paintBackground(juce::Graphics& g)
{
	int zoomedCheckerSize = checkerSize * this->manager->viewZoom;

	int checkerTX = -zoomedCheckerSize * 2 + ((this->getWidth() / 2 + this->manager->viewOffset.x) % (zoomedCheckerSize * 2));
	int checkerTY = -zoomedCheckerSize * 2 + ((this->getHeight() / 2 + this->manager->viewOffset.y) % (zoomedCheckerSize * 2));
	juce::Rectangle<int> checkerBounds(checkerTX, checkerTY, this->getWidth() + zoomedCheckerSize * 4, this->getHeight() + zoomedCheckerSize * 4);
	g.fillCheckerBoard(checkerBounds.toFloat(), zoomedCheckerSize, zoomedCheckerSize, BG_COLOR.darker(.3f), BG_COLOR.darker(.2f));

	g.setColour(BG_COLOR.darker(.05f));
	juce::Point<int> center = getSize() / 2;
	center += this->manager->viewOffset;
	g.drawLine(center.x, 0, center.x, this->getHeight(), 2);
	g.drawLine(0, center.y, this->getWidth(), center.y, 2);

	if (this->manager->showSnapGrid != nullptr && this->manager->showSnapGrid->boolValue())
	{
		g.setColour(BG_COLOR.brighter(.1f));

		float step = this->manager->snapGridSize->intValue() * this->manager->viewZoom;
		float startX = fmodf(this->getViewCenter().x, step) - step;
		float startY = fmodf(this->getViewCenter().y, step) - step;
		for (float i = startX; i <= this->getWidth(); i += step) g.drawVerticalLine(i, 0, this->getHeight());
		for (float i = startY; i <= this->getHeight(); i += step) g.drawHorizontalLine(i, 0, this->getWidth());
	}
}

template<class M, class T, class U>
inline void BaseManagerViewUI<M, T, U>::paintOverChildren(juce::Graphics& g)
{
	if (this->inspectable.wasObjectDeleted()) return;

	BaseManagerUI<M, T, U>::paintOverChildren(g);

	if (snapLineX.getLength() > 0)
	{
		g.setColour(HIGHLIGHT_COLOR);
		float dl[] = { 4,2 };
		g.drawDashedLine(snapLineX.toFloat(), dl, 2, 1);
	}

	if (snapLineY.getLength() > 0)
	{
		g.setColour(HIGHLIGHT_COLOR);
		float dl[] = { 4,2 };
		g.drawDashedLine(snapLineY.toFloat(), dl, 2, 1);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::resized()
{
	if (this->inspectable.wasObjectDeleted()) return;

	juce::Rectangle<int> r = this->getLocalBounds();
	//this->addItemBT->setBounds(r.withSize(24, 24).withX(r.getWidth() - 24));

	juce::Rectangle<int> hr = r.removeFromTop(this->headerSize);
	this->resizedInternalHeader(hr);

	juce::Array<U*> filteredItems = this->getFilteredItems();
	for (auto& tui : filteredItems)
	{
		updateViewUIPosition(tui);
	}

	if (viewPane != nullptr)
	{
		int size = juce::jlimit(50, 300, juce::jmax(r.getWidth() / 5, r.getHeight() / 5) - 20);
		viewPane->setBounds(r.translated(-10, -10).removeFromRight(size).removeFromBottom(size));
	}

	updateItemsVisibility();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateViewUIPosition(U* itemUI)
{
	if (itemUI == nullptr) return;
	updateComponentViewPosition((juce::Component*)itemUI, itemUI->item->viewUIPosition->getPoint(), getUITransform(itemUI));
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateComponentViewPosition(juce::Component* c, juce::Point<float> position, const juce::AffineTransform& af)
{
	if (c == nullptr) return;

	juce::Point<int> p = getPosInView(position).toInt();

	if (centerUIAroundPosition) p -= (juce::Point<int>(c->getWidth(), c->getHeight()) * this->manager->viewZoom) / 2;
	c->setTopLeftPosition(p.x, p.y);

	juce::AffineTransform t = af;
	if (zoomAffectsItemSize)
	{
		t = af.scaled(this->manager->viewZoom, this->manager->viewZoom, p.x, p.y);
	}
	c->setTransform(t);
}

template<class M, class T, class U>
inline juce::AffineTransform BaseManagerViewUI<M, T, U>::getUITransform(U* itemUI)
{
	return juce::AffineTransform();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateItemsVisibility()
{
	//BaseManagerUI::updateItemsVisibility();

	bool hasChanged = false;

	juce::Rectangle<int> r = this->getLocalBounds();
	for (auto& iui : this->itemsUI)
	{
		if (!this->checkFilterForItem(iui))
		{
			if (iui->isVisible())
			{
				iui->setVisible(false);
				hasChanged = true;
			}
			continue;
		};

		if (!this->checkItemShouldBeVisible(iui))
		{
			if (iui->isVisible())
			{
				iui->setVisible(false);
				hasChanged = true;
			}
			continue;
		}

		juce::Rectangle<int> iuiB = iui->getBoundsInParent();
		juce::Rectangle<int> ir = iuiB.getIntersection(r);
		bool isInsideInspectorBounds = !ir.isEmpty();
		if (iui->isVisible() != isInsideInspectorBounds)
		{
			iui->setVisible(isInsideInspectorBounds);
			hasChanged = true;
		}
	}

	if (hasChanged && viewPane != nullptr)
	{
		viewPane->updateContent();
		viewPane->toFront(false);
	}

	if (this->toolContainer.isVisible()) this->toolContainer.toFront(false);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::parentHierarchyChanged()
{
	updateItemsVisibility();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemFromMenu(bool isFromAddButton, juce::Point<int> mouseDownPos)
{
	addItemFromMenu(nullptr, isFromAddButton, mouseDownPos);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemFromMenu(T* item, bool isFromAddButton, juce::Point<int> mouseDownPos)
{
	this->manager->addItem(item, isFromAddButton ? juce::Point<float>(0, 0) : getViewPos(mouseDownPos).toFloat());
}

template<class M, class T, class U>
juce::Point<int> BaseManagerViewUI<M, T, U>::getSize()
{
	return juce::Point<int>(this->getWidth(), this->getHeight());
}

template<class M, class T, class U>
juce::Point<float> BaseManagerViewUI<M, T, U>::getViewMousePosition()
{
	return getViewPos(this->getMouseXYRelative());
}

template<class M, class T, class U>
juce::Point<float> BaseManagerViewUI<M, T, U>::getViewPos(const juce::Point<int>& originalPos)
{
	return (originalPos - getViewCenter()).toFloat() / (this->manager->viewZoom * (float)(useCheckersAsUnits ? checkerSize : 1));
}

template<class M, class T, class U>
inline juce::Point<float> BaseManagerViewUI<M, T, U>::getViewOffset(const juce::Point<int>& offsetInView)
{
	return offsetInView.toFloat() / (this->manager->viewZoom * (useCheckersAsUnits ? checkerSize : 1));
}

template<class M, class T, class U>
juce::Rectangle<float> BaseManagerViewUI<M, T, U>::getViewBounds(const juce::Rectangle<int>& r)
{
	const float checkerMultiplier = useCheckersAsUnits ? checkerSize : 1;
	return juce::Rectangle<float>().withPosition(getViewPos(r.getPosition())).withSize(r.getWidth() / (this->manager->viewZoom * checkerMultiplier), r.getHeight() / (this->manager->viewZoom * checkerMultiplier));
}

template<class M, class T, class U>
juce::Point<int> BaseManagerViewUI<M, T, U>::getViewCenter()
{
	return this->manager->viewOffset + (getSize() / 2);
}

template<class M, class T, class U>
juce::Point<int> BaseManagerViewUI<M, T, U>::getPosInView(const juce::Point<float>& viewPos)
{
	return (viewPos * this->manager->viewZoom * (float)(useCheckersAsUnits ? checkerSize : 1)).toInt() + getViewCenter();
}

template<class M, class T, class U>
juce::Rectangle<int> BaseManagerViewUI<M, T, U>::getBoundsInView(const juce::Rectangle<float>& r)
{
	const float checkerMultiplier = useCheckersAsUnits ? checkerSize : 1;
	return juce::Rectangle<int>().withPosition(getPosInView(r.getPosition())).withSize(r.getWidth() * this->manager->viewZoom * checkerMultiplier, r.getHeight() * this->manager->viewZoom * checkerMultiplier);
}

template<class M, class T, class U>
juce::Point<float> BaseManagerViewUI<M, T, U>::getItemsCenter()
{
	if (this->manager->items.size() == 0) return juce::Point<float>(0, 0);

	juce::Rectangle<float> bounds;
	for (auto& i : this->manager->items)
	{
		juce::Point<float> p1 = i->viewUIPosition->getPoint();
		juce::Point<float> p2 = p1 + i->viewUISize->getPoint();
		juce::Rectangle<float> r(p1, p2);
		if (bounds.isEmpty()) bounds = r;
		else bounds = bounds.getUnion(r);
	}

	return bounds.getCentre();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::homeView()
{
	this->manager->viewOffset = juce::Point<int>();

	this->resized();
	this->repaint();

	updateItemsVisibility();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::frameView(U* se)
{
	if (se == nullptr) this->manager->viewOffset = -getItemsCenter().toInt() * this->manager->viewZoom;
	else this->manager->viewOffset = -(se->item->viewUIPosition->getPoint() + se->item->viewUISize->getPoint() / 2).toInt() * this->manager->viewZoom;

	this->resized();
	this->repaint();

	updateItemsVisibility();

}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::setViewZoom(float value)
{
	if (this->manager->viewZoom == value) return;
	this->manager->viewZoom = juce::jlimit<float>(minZoom, maxZoom, value);
	for (auto& tui : this->itemsUI) tui->setViewZoom(this->manager->viewZoom);

	updateItemsVisibility();
	this->resized();
	this->repaint();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::setShowPane(bool val)
{
	bool showingPane = viewPane != nullptr;
	if (val == showingPane) return;
	if (val)
	{
		viewPane.reset(createViewPane());
		this->addAndMakeVisible(viewPane.get());
	}
	else
	{
		this->removeChildComponent(viewPane.get());
		viewPane.reset(nullptr);
	}
}

template<class M, class T, class U>
BaseManagerViewMiniPane<M, T, U>* BaseManagerViewUI<M, T, U>::createViewPane()
{
	return new BaseManagerViewMiniPane<M, T, U>(this);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemUIInternal(U* se)
{
	se->setViewZoom(this->manager->viewZoom);
	if (useCheckersAsUnits) se->setViewCheckerSize(checkerSize);
	updateViewUIPosition(se);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
	BaseManagerUI<M, T, U>::itemDragMove(dragSourceDetails);

	BaseItemMinimalUI<T>* bui = dynamic_cast<BaseItemMinimalUI<T>*>(dragSourceDetails.sourceComponent.get());
	if (bui == nullptr) return;

	juce::Point<int> relOffset = juce::Point<int>((int)dragSourceDetails.description.getProperty("offsetX", 0), (int)dragSourceDetails.description.getProperty("offsetY", 0));
	juce::Point<int> realP = this->getMouseXYRelative() - (this->getLocalPoint(bui, relOffset) - bui->getPosition()) * 1.0f / this->manager->viewZoom;


	juce::Point<int> snapPosition = realP;

	if (this->manager->snapGridMode != nullptr && this->manager->snapGridMode->boolValue())
	{
		juce::Point<float> vPos = this->getViewPos(snapPosition);
		float snapViewSize = this->manager->snapGridSize->floatValue() / (useCheckersAsUnits ? checkerSize : 1);
		targetSnapViewPosition.setXY(vPos.x - fmodf(vPos.x, snapViewSize), vPos.y - fmodf(vPos.y, snapViewSize));
		snapPosition = this->getPosInView(targetSnapViewPosition);
	}
	else if (enableSnapping)
	{

		int distX = snappingThreshold;
		int distY = snappingThreshold;

		int targetX = 0;
		int targetY = 0;

		juce::Rectangle<int> sb = bui->getBounds().withPosition(realP);

		BaseItemMinimalUI<T>* targetUIX = nullptr;
		BaseItemMinimalUI<T>* targetUIY = nullptr;

		bool snapIsLeft = false;
		bool snapIsTop = false;

		for (auto& ui : this->itemsUI)
		{
			if (ui->baseItem->isSelected) continue;
			juce::Rectangle<int> ib = ui->getBounds();

			int curDistX = distX;
			int curDistY = distY;

			int leftLeft = abs(sb.getX() - ib.getX());
			int leftRightSpaced = abs(sb.getX() - (ib.getRight() + snappingSpacing));
			int rightRight = abs(sb.getRight() - ib.getRight());
			int rightLeftSpaced = abs(sb.getRight() - (ib.getX() - snappingSpacing));

			if (leftLeft < curDistX) { curDistX = leftLeft; targetX = ib.getX(); snapIsLeft = true; }
			if (leftRightSpaced < curDistX) { curDistX = leftRightSpaced; targetX = ib.getRight() + snappingSpacing; snapIsLeft = true; }
			if (rightRight < curDistX) { curDistX = rightRight; targetX = ib.getRight(); snapIsLeft = false; }
			if (rightLeftSpaced < curDistX) { curDistX = rightLeftSpaced; targetX = ib.getX() - snappingSpacing; snapIsLeft = false; }


			int topTop = abs(sb.getY() - ib.getY());
			int topBottomSpaced = abs(sb.getY() - (ib.getBottom() + snappingSpacing));
			int bottomBottom = abs(sb.getBottom() - ib.getBottom());
			int bottomTopSpaced = abs(sb.getBottom() - (ib.getY() - snappingSpacing));

			if (topTop < curDistY) { curDistY = topTop; targetY = ib.getY(); snapIsTop = true; }
			if (topBottomSpaced < curDistY) { curDistY = topBottomSpaced; targetY = ib.getBottom() + snappingSpacing; snapIsTop = true; }
			if (bottomBottom < curDistY) { curDistY = bottomBottom; targetY = ib.getBottom(); snapIsTop = false; }
			if (bottomTopSpaced < curDistY) { curDistY = bottomTopSpaced; targetY = ib.getY() - snappingSpacing; snapIsTop = false; }

			if (curDistX < distX)
			{
				targetUIX = dynamic_cast<BaseItemMinimalUI<T>*>(ui);
				distX = curDistX;
			}

			if (curDistY < distY)
			{
				targetUIY = dynamic_cast<BaseItemMinimalUI<T>*>(ui);
				distY = curDistY;
			}
		}


		snapLineX = juce::Line<int>();
		if (targetUIX != nullptr)
		{
			snapLineX.setStart(juce::Point<int>(targetX, juce::jmin<int>(sb.getY(), targetUIX->getBounds().getY()) - 20));
			snapLineX.setEnd(juce::Point<int>(targetX, juce::jmax<int>(sb.getBottom(), targetUIX->getBounds().getBottom()) + 20));
			snapPosition.setX(snapIsLeft ? targetX : targetX - bui->getWidth());
		}

		snapLineY = juce::Line<int>();
		if (targetUIY != nullptr)
		{
			snapLineY.setStart(juce::Point<int>(juce::jmin<int>(sb.getX(), targetUIY->getBounds().getX()) - 20, targetY));
			snapLineY.setEnd(juce::Point<int>(juce::jmax<int>(sb.getRight(), targetUIY->getBounds().getRight()) + 20, targetY));
			snapPosition.setY(snapIsTop ? targetY : targetY - bui->getHeight());
		}

		targetSnapViewPosition = this->getViewPos(snapPosition);

	}

	if (updatePositionOnDragMove)
	{
		juce::Point<float> targetPosition = (snapPosition != realP) ? targetSnapViewPosition : this->getPositionFromDrag(bui, dragSourceDetails);
		if (juce::Desktop::getInstance().getMainMouseSource().getCurrentModifiers().isAltDown()) bui->baseItem->scalePosition(targetPosition - bui->baseItem->movePositionReference, true);
		else bui->baseItem->movePosition(targetPosition - bui->baseItem->movePositionReference, true);
	}

	this->repaint();

}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
	BaseManagerUI<M, T, U>::itemDropped(dragSourceDetails);

	BaseItemMinimalUI<T>* bui = dynamic_cast<BaseItemMinimalUI<T> *>(dragSourceDetails.sourceComponent.get());

	if (bui != nullptr && this->itemsUI.contains((U*)bui))
	{
		juce::Point<float> p = enableSnapping ? targetSnapViewPosition : this->getPositionFromDrag(bui, dragSourceDetails);
		if (juce::Desktop::getInstance().getMainMouseSource().getCurrentModifiers().isAltDown()) bui->baseItem->scalePosition(p - bui->baseItem->movePositionReference, true);
		else bui->baseItem->movePosition(p - bui->baseItem->movePositionReference, true);
		bui->baseItem->addMoveToUndoManager(true);
	}

	snapLineX = juce::Line<int>();
	snapLineY = juce::Line<int>();
	this->repaint();
}

template<class M, class T, class U>
juce::Point<float> BaseManagerViewUI<M, T, U>::getPositionFromDrag(juce::Component* c, const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
	juce::Point<int> relOffset = juce::Point<int>((int)dragSourceDetails.description.getProperty("offsetX", 0), (int)dragSourceDetails.description.getProperty("offsetY", 0));
	juce::Point<int> realP = this->getMouseXYRelative() - (this->getLocalPoint(c, relOffset) - c->getPosition()) * 1.0f / this->manager->viewZoom;
	if (centerUIAroundPosition) realP += (juce::Point<int>(c->getWidth(), c->getHeight()) * this->manager->viewZoom) / 2;
	return this->getViewPos(realP);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemUIMiniModeChanged(BaseItemUI<T>* itemUI)
{
	updateViewUIPosition(dynamic_cast<U*>(itemUI));
}




template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemUIViewPositionChanged(BaseItemMinimalUI<T>* itemUI)
{
	updateViewUIPosition(dynamic_cast<U*>(itemUI));
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::askForSyncPosAndSize(BaseItemMinimalUI<T>* itemUI)
{
	juce::Array<juce::UndoableAction*> actions;
	actions.addArray(itemUI->baseItem->viewUIPosition->setUndoablePoint(getViewPos(itemUI->getPosition()).toFloat(), true));
	actions.addArray(itemUI->baseItem->viewUISize->setUndoablePoint(juce::Point<float>(itemUI->getWidth(), itemUI->getHeight()), true));
	UndoMaster::getInstance()->performActions("Move / Resize " + itemUI->baseItem->niceName, actions);
}


template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemUIResizeDrag(BaseItemMinimalUI<T>* itemUI, const juce::Point<int>& dragOffset)
{
	juce::Point<float> pos = itemUI->baseItem->getPosition() + itemUI->baseItem->sizeReference + dragOffset.toFloat() / (useCheckersAsUnits ? checkerSize : 1);// getViewOffset(dragOffset);

	juce::Point<float> snapPos = pos;

	if (this->manager->snapGridMode != nullptr && this->manager->snapGridMode->boolValue())
	{
		float snapViewSize = this->manager->snapGridSize->floatValue() / (useCheckersAsUnits ? checkerSize : 1);
		snapPos.setXY(pos.x - fmodf(pos.x, snapViewSize), pos.y - fmodf(pos.y, snapViewSize));
	}
	else if (enableSnapping)
	{

		SnapResult snapX = getClosestSnapUI(pos, true, snappingSpacing, 0);
		SnapResult snapY = getClosestSnapUI(pos, false, snappingSpacing, 0);

		juce::Point<int> snapInView = getPosInView(juce::Point<float>(snapX.pos, snapY.pos));

		snapLineX = juce::Line<int>();
		if (snapX.targetUI != nullptr)
		{
			snapLineX.setStart(juce::Point<int>(snapInView.x, juce::jmin<int>(itemUI->getBounds().getY(), snapX.targetUI->getBounds().getY()) - 20));
			snapLineX.setEnd(juce::Point<int>(snapInView.x, juce::jmax<int>(itemUI->getBounds().getBottom(), snapX.targetUI->getBounds().getBottom()) + 20));
			snapPos.x = snapX.pos;
		}

		snapLineY = juce::Line<int>();
		if (snapY.targetUI != nullptr)
		{
			snapLineY.setStart(juce::Point<int>(juce::jmin<int>(itemUI->getBounds().getX(), snapY.targetUI->getBounds().getX()) - 20, snapInView.y));
			snapLineY.setEnd(juce::Point<int>(juce::jmax<int>(itemUI->getBounds().getRight(), snapY.targetUI->getBounds().getRight()) + 20, snapInView.y));
			snapPos.y = snapY.pos;
		}

		this->repaint();
	}


	juce::Point<float> offset = snapPos - (itemUI->baseItem->getPosition() + itemUI->baseItem->sizeReference);
	itemUI->baseItem->resizeItem(offset, true);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemUIResizeEnd(BaseItemMinimalUI<T>* itemUI)
{
	snapLineX = juce::Line<int>();
	snapLineY = juce::Line<int>();

	if (itemUI->baseItem->viewUISize->x <= 0 || itemUI->baseItem->viewUISize->y <= 0)
	{
		itemUI->baseItem->viewUISize->setPoint(itemUI->baseItem->sizeReference);
	}
	else
	{
		itemUI->baseItem->addResizeToUndoManager(true);
	}


	this->repaint();
}

template<class M, class T, class U>
typename BaseManagerViewUI<M, T, U>::SnapResult BaseManagerViewUI<M, T, U>::getClosestSnapUI(juce::Point<float> pos, bool isX, float spacingBefore, float spacingAfter)
{
	int dist = snappingThreshold;
	int target = 0;

	BaseItemMinimalUI<T>* targetUI = nullptr;

	float initPos = isX ? pos.x : pos.y;

	for (auto& ui : this->itemsUI)
	{
		if (ui->baseItem->isSelected) continue;
		juce::Rectangle<float> ib = getViewBounds(ui->getBounds());

		int curDist = dist;

		float posBefore = (isX ? ib.getX() : ib.getY()) - spacingBefore;
		float posAfter = (isX ? ib.getRight() : ib.getBottom()) + spacingAfter;

		float distBefore = fabsf(initPos - posBefore);
		float distAfter = fabsf(initPos - posAfter);

		if (distBefore < curDist) { curDist = distBefore; target = posBefore; }
		if (distAfter < curDist) { curDist = distAfter; target = posAfter; }

		if (curDist < dist)
		{
			targetUI = dynamic_cast<BaseItemMinimalUI<T>*>(ui);
			dist = curDist;
		}
	}

	return SnapResult(targetUI, target);
}
