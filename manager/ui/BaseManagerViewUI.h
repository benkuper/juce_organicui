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
	public BaseManagerUI<M, T, U>,
	public Engine::AsyncListener
{
public:
	BaseManagerViewUI<M, T, U>(const String &contentName, M * _manager);
	virtual ~BaseManagerViewUI();

	bool canNavigate;

	//
	bool centerUIAroundPosition;
	bool updatePositionOnDragMove;
	bool enableSnapping;
	const int snappingThreshold = 10;
	const int snappingSpacing = 10;
	Line<int> snapLineX;
	Line<int> snapLineY;
	Point<float> targetSnapViewPosition;

	//Checkers
	bool useCheckersAsUnits;
	int checkerSize = 128;

	//Zoom
	bool canZoom;
	bool zoomAffectsItemSize;
	float viewZoom;
	float minZoom;
	float maxZoom;

	double timeSinceLastWheel;

	Point<int> viewOffset; //in pixels, viewOffset of 0 means zeroPos is at the center of the window
						   //interaction
	Point<int> initViewOffset;



	std::unique_ptr<BaseManagerViewMiniPane<M, T, U>> viewPane;

	virtual void mouseDown(const MouseEvent &e) override;
	virtual void mouseDrag(const MouseEvent &e) override;
	virtual void mouseUp(const MouseEvent &e) override;
	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& d) override;
	void mouseMagnify(const MouseEvent &e, float scaleFactor) override;
	virtual bool keyPressed(const KeyPress &e) override;

	virtual void paint(Graphics &g) override;
	virtual void paintBackground(Graphics &g);
	virtual void paintOverChildren(Graphics& g) override;

	virtual void resized() override;

	virtual void updateViewUIPosition(U * se);
	virtual void updateComponentViewPosition(Component* c, Point<float> position, const AffineTransform &af);
	virtual AffineTransform getUITransform(U * se);

	virtual void updateItemsVisibility() override;

	virtual void parentHierarchyChanged() override;

	virtual void addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos) override;
	virtual void addItemFromMenu(T * item, bool isFromAddButton, Point<int> mouseDownPos) override;

	Point<int> getSize();
	Point<float> getViewMousePosition();
	Point<float> getViewPos(const Point<int>& originalPos);
	Point<float> getViewOffset(const Point<int>& offsetInView);
	juce::Rectangle<float> getViewBounds(const juce::Rectangle<int> &originalBounds);
	Point<int> getViewCenter();
	Point<int> getPosInView(const Point<float> &viewPos);
	juce::Rectangle<int> getBoundsInView(const juce::Rectangle<float> &r);
	Point<float> getItemsCenter();

	virtual void homeView();
	virtual void frameView(U * item = nullptr);

	virtual void setViewZoom(float newZoom);
	virtual void setShowPane(bool val);
	virtual BaseManagerViewMiniPane<M, T, U>* createViewPane();

	virtual void addItemUIInternal(U * se) override; 

	//virtual void itemUIGrabbed(BaseItemMinimalUI<T> * se) override;

	void itemDragMove(const DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const DragAndDropTarget::SourceDetails &dragSourceDetails) override;

	virtual Point<float> getPositionFromDrag(Component * c, const DragAndDropTarget::SourceDetails& dragSourceDetails);

	virtual void itemUIMiniModeChanged(BaseItemUI<T> * itemUI) override;
	virtual void itemUIViewPositionChanged(BaseItemMinimalUI<T> * itemUI) override;
	virtual void askForSyncPosAndSize(BaseItemMinimalUI<T>* itemUI) override;

	virtual void newMessage(const Engine::EngineEvent& e) override;


};



template<class M, class T, class U>
BaseManagerViewUI<M, T, U>::BaseManagerViewUI(const String& contentName, M* _manager) :
	BaseManagerUI<M, T, U>(contentName, _manager, false),
	canNavigate(true),
	centerUIAroundPosition(false),
	updatePositionOnDragMove(false),
	enableSnapping(false),
    useCheckersAsUnits(false),
	canZoom(true),
	zoomAffectsItemSize(true),
	viewZoom(1),
	minZoom(.4f),
	maxZoom(1),
	timeSinceLastWheel(0)
{
    this->defaultLayout = this->FREE;

	this->resizeOnChildBoundsChanged = false;
	this->bgColor = BG_COLOR.darker(.3f);

	Engine::mainEngine->addAsyncEngineListener(this);
}


template<class M, class T, class U>
BaseManagerViewUI<M, T, U>::~BaseManagerViewUI()
{
	Engine::mainEngine->removeAsyncEngineListener(this);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseDown(const MouseEvent & e)
{
	BaseManagerUI<M, T, U>::mouseDown(e);
	if (canNavigate && ((e.mods.isLeftButtonDown() && e.mods.isAltDown()) || e.mods.isMiddleButtonDown()))
	{
		this->setMouseCursor(MouseCursor::UpDownLeftRightResizeCursor);
		this->updateMouseCursor();
		initViewOffset = Point<int>(viewOffset.x, viewOffset.y);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseDrag(const MouseEvent & e)
{
	BaseManagerUI<M, T, U>::mouseDrag(e);

	if (canNavigate && ((e.mods.isLeftButtonDown() && e.mods.isAltDown()) || e.mods.isMiddleButtonDown()))
	{
		viewOffset = initViewOffset + e.getOffsetFromDragStart();
		updateItemsVisibility();
		this->resized();
		this->repaint();
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::mouseUp(const MouseEvent & e)
{
	BaseManagerUI<M, T, U>::mouseUp(e);
	
	snapLineX = Line<int>();
	snapLineY = Line<int>();

	this->setMouseCursor(MouseCursor::NormalCursor);
	this->updateMouseCursor();
}

template<class M, class T, class U>
 void BaseManagerViewUI<M, T, U>::mouseWheelMove(const MouseEvent & e, const MouseWheelDetails & d)
{
	 
	 if (e.mods.isShiftDown())
	 {
		 if (canZoom)
		 {
			 Point<float> curMousePos = getViewMousePosition();
			 
			 setViewZoom(viewZoom + d.deltaY);
			 
			 Point<float> newMousePos = getViewMousePosition();
			 viewOffset += ((newMousePos - curMousePos) * viewZoom * (useCheckersAsUnits ? checkerSize : 1)).toInt();
			 updateItemsVisibility();
			 this->resized();
		 }
	 }
	 else
	 {
		 double curTime = Time::getApproximateMillisecondCounter() / 1000.0;
		 bool newCheck = curTime - timeSinceLastWheel > 1;

		 if (newCheck && e.originalComponent != this) return;
		 
		 float sensitivity = 500;
		 viewOffset += Point<int>(d.deltaX*sensitivity, d.deltaY*sensitivity);
		 updateItemsVisibility(); 
		 this->resized();
		 this->repaint();

		 timeSinceLastWheel = curTime;
	 }
	

}

 template<class M, class T, class U>
 void BaseManagerViewUI<M, T, U>::mouseMagnify(const MouseEvent& e, float scaleFactor)
 {
	 if (canZoom)
	 {
		 Point<float> curMousePos = getViewMousePosition();

		 setViewZoom(viewZoom + scaleFactor);

		 Point<float> newMousePos = getViewMousePosition();
		 viewOffset += ((newMousePos - curMousePos)  *viewZoom).toInt();
		 this->resized();
	 }
 }

template<class M, class T, class U>
bool BaseManagerViewUI<M, T, U>::keyPressed(const KeyPress & e)
{
	if (BaseManagerUI<M, T, U>::keyPressed(e)) return true;

	if (e.getKeyCode() == KeyPress::createFromDescription("f").getKeyCode())
	{
		frameView();
		return true;
	} else if (e.getKeyCode() == KeyPress::createFromDescription("h").getKeyCode())
	{
		homeView();
		return true;
	}

	return false;
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::paint(Graphics & g)
{
	if(!this->transparentBG) paintBackground(g);

	if (this->manager->items.size() == 0 && this->noItemText.isNotEmpty())
	{
		g.setColour(Colours::white.withAlpha(.4f));
		g.setFont(16);
		g.drawFittedText(this->noItemText, this->getLocalBounds(), Justification::centred, 6);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::paintBackground(Graphics & g)
{
	int zoomedCheckerSize = checkerSize * viewZoom;

	int checkerTX = -zoomedCheckerSize * 2 + ((this->getWidth() / 2 + viewOffset.x) % (zoomedCheckerSize * 2));
	int checkerTY = -zoomedCheckerSize * 2 + ((this->getHeight() / 2 + viewOffset.y) % (zoomedCheckerSize * 2));
	juce::Rectangle<int> checkerBounds(checkerTX, checkerTY, this->getWidth() + zoomedCheckerSize * 4, this->getHeight() + zoomedCheckerSize * 4);
	g.fillCheckerBoard(checkerBounds.toFloat(), zoomedCheckerSize, zoomedCheckerSize, BG_COLOR.darker(.3f), BG_COLOR.darker(.2f));

	g.setColour(BG_COLOR.darker(.05f));
	Point<int> center = getSize() / 2;
	center += viewOffset;
	g.drawLine(center.x, 0, center.x, this->getHeight(), 2);
	g.drawLine(0, center.y, this->getWidth(), center.y, 2);

}

template<class M, class T, class U>
inline void BaseManagerViewUI<M, T, U>::paintOverChildren(Graphics& g)
{
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
	juce::Rectangle<int> r = this->getLocalBounds();
	this->addItemBT->setBounds(r.withSize(24, 24).withX(r.getWidth() - 24));
	for (auto &tui : this->itemsUI)
	{
		updateViewUIPosition(tui);
	}

	if (viewPane != nullptr)
	{
		int size = jlimit(50,200, jmin(r.getWidth()/5, r.getHeight()/5) - 20);
		viewPane->setBounds(r.translated(-10, -10).removeFromRight(size).removeFromBottom(size));
	}

	updateItemsVisibility();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateViewUIPosition(U * itemUI)
{	
	if (itemUI == nullptr) return;
	updateComponentViewPosition((Component*)itemUI, itemUI->item->viewUIPosition->getPoint(), getUITransform(itemUI));
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateComponentViewPosition(Component* c, Point<float> position, const AffineTransform &af)
{
	if (c == nullptr) return;

	Point<int> p = getPosInView(position).toInt();

	if (centerUIAroundPosition) p -= (Point<int>(c->getWidth(), c->getHeight()) * viewZoom) / 2;
	c->setTopLeftPosition(p.x, p.y);

	AffineTransform t = af;
	if (zoomAffectsItemSize)
	{
		t = af.scaled(viewZoom, viewZoom, p.x, p.y);
	}
	c->setTransform(t);
}

template<class M, class T, class U>
inline AffineTransform BaseManagerViewUI<M, T, U>::getUITransform(U* itemUI)
{
	return AffineTransform();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateItemsVisibility()
{
	//BaseManagerUI::updateItemsVisibility();
	juce::Rectangle<int> r = this->getLocalBounds();
	for (auto& iui : this->itemsUI)
	{
		juce::Rectangle<int> iuiB = iui->getBoundsInParent();
		juce::Rectangle<int> ir = iuiB.getIntersection(r);
		bool isInsideInspectorBounds = !ir.isEmpty();
		iui->setVisible(isInsideInspectorBounds);
	}

	if (viewPane != nullptr)
	{
		viewPane->updateContent();
		viewPane->toFront(false);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::parentHierarchyChanged()
{
	updateItemsVisibility();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos)
{
	addItemFromMenu(nullptr, isFromAddButton, mouseDownPos);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemFromMenu(T * item, bool isFromAddButton, Point<int> mouseDownPos)
{
	this->manager->addItem(item, isFromAddButton?Point<float>(0,0):getViewPos(mouseDownPos).toFloat());
}

template<class M, class T, class U>
Point<int> BaseManagerViewUI<M, T, U>::getSize()
{
	return Point<int>(this->getWidth(), this->getHeight());
}

template<class M, class T, class U>
Point<float> BaseManagerViewUI<M, T, U>::getViewMousePosition()
{
	return getViewPos(this->getMouseXYRelative());
}

template<class M, class T, class U>
Point<float> BaseManagerViewUI<M, T, U>::getViewPos(const Point<int>& originalPos)
{
	return (originalPos - getViewCenter()).toFloat() / (viewZoom * (float)(useCheckersAsUnits?checkerSize:1));
}

template<class M, class T, class U>
inline Point<float> BaseManagerViewUI<M, T, U>::getViewOffset(const Point<int>& offsetInView)
{
	return offsetInView.toFloat() / (viewZoom * (useCheckersAsUnits ? checkerSize : 1));
}

template<class M, class T, class U>
juce::Rectangle<float> BaseManagerViewUI<M, T, U>::getViewBounds(const juce:: Rectangle<int>& r)
{
	const float checkerMultiplier = useCheckersAsUnits ? checkerSize : 1;
	return juce::Rectangle<float>().withPosition(getViewPos(r.getPosition())).withSize(r.getWidth() / (viewZoom * checkerMultiplier), r.getHeight() / (viewZoom * checkerMultiplier));
}

template<class M, class T, class U>
 Point<int> BaseManagerViewUI<M, T, U>::getViewCenter()
{
	return viewOffset + (getSize() / 2);
}

template<class M, class T, class U>
Point<int> BaseManagerViewUI<M, T, U>::getPosInView(const Point<float>& viewPos)
{
	return (viewPos * viewZoom * (float)(useCheckersAsUnits?checkerSize:1)).toInt() + getViewCenter();
}

template<class M, class T, class U>
 juce::Rectangle<int> BaseManagerViewUI<M, T, U>::getBoundsInView(const juce::Rectangle<float>& r)
{
	 const float checkerMultiplier = useCheckersAsUnits ? checkerSize : 1;
	 return juce::Rectangle<int>().withPosition(getPosInView(r.getPosition())).withSize(r.getWidth() * viewZoom * checkerMultiplier, r.getHeight() * viewZoom * checkerMultiplier);
}

template<class M, class T, class U>
Point<float> BaseManagerViewUI<M, T, U>::getItemsCenter()
{
	if (this->itemsUI.size() == 0) return Point<float>(0, 0);

	juce::Rectangle<float> bounds;
	for (auto &se : this->itemsUI)
	{
		Point<float> p1 = se->item->viewUIPosition->getPoint();
		Point<float> p2 = p1+se->item->viewUISize->getPoint();
		bounds = juce::Rectangle<float>(Point<float>(jmin(bounds.getX(), p1.x), jmin(bounds.getY(), p1.y)), Point<float>(jmax<float>(bounds.getRight(), p2.x), jmax<float>(bounds.getBottom(), p2.y)));
	}
	return bounds.getCentre();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::homeView()
{
	viewOffset = Point<int>();

	this->resized();
	this->repaint();

	updateItemsVisibility();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::frameView(U* se)
{
	if (se == nullptr) viewOffset = -getItemsCenter().toInt() * viewZoom;
	else viewOffset = -(se->item->viewUIPosition->getPoint() + se->item->viewUISize->getPoint() / 2).toInt() * viewZoom;

	this->resized();
	this->repaint();

	updateItemsVisibility();

}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::setViewZoom(float value)
{
	if (viewZoom == value) return;
	viewZoom = jlimit<float>(minZoom, maxZoom, value);
	for (auto &tui : this->itemsUI) tui->setViewZoom(viewZoom);

	updateItemsVisibility();
	this->resized();
	this->repaint();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::setShowPane(bool val)
{
	bool showingPane = viewPane != nullptr;
	if (val == showingPane) return;
	if(val)
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
void BaseManagerViewUI<M, T, U>::addItemUIInternal(U * se)
{
	se->setViewZoom(viewZoom);
	if (useCheckersAsUnits) se->setViewCheckerSize(checkerSize); 
	updateViewUIPosition(se);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemDragMove(const DragAndDropTarget::SourceDetails& dragSourceDetails)
{
	BaseManagerUI<M, T, U>::itemDragMove(dragSourceDetails);

	BaseItemMinimalUI<T>* bui = dynamic_cast<BaseItemMinimalUI<T>*>(dragSourceDetails.sourceComponent.get());
	if (bui == nullptr) return;
	
	Point<int> relOffset = Point<int>((int)dragSourceDetails.description.getProperty("offsetX", 0), (int)dragSourceDetails.description.getProperty("offsetY", 0));
	Point<int> realP = this->getMouseXYRelative() - (this->getLocalPoint(bui, relOffset) - bui->getPosition()) *1.0f / viewZoom;


	Point<int> snapPosition = realP;

	if (enableSnapping)
	{
		
		int distX = snappingThreshold;
		int distY = snappingThreshold;

		int targetX = 0;
		int targetY = 0;
		
		juce::Rectangle<int> sb = bui->getBounds().withPosition(realP);

		BaseItemMinimalUI<T> * targetUIX = nullptr;
		BaseItemMinimalUI<T> * targetUIY = nullptr;

		bool snapIsLeft = false;
		bool snapIsTop = false;

		for (auto& ui : this->itemsUI)
		{
			if (ui == bui) continue;
			juce::Rectangle<int> ib = ui->getBounds();
			
			int curDistX = distX;
			int curDistY = distY;

			int leftLeft = abs(sb.getX() - ib.getX());
			int leftRightSpaced = abs(sb.getX() - (ib.getRight() + snappingSpacing));
			int rightRight = abs(sb.getRight() - ib.getRight());
			int rightLeftSpaced = abs(sb.getRight() - (ib.getX() - snappingSpacing));

			if (leftLeft < curDistX) { curDistX = leftLeft; targetX = ib.getX(); snapIsLeft = true; }
			if (leftRightSpaced < curDistX) { curDistX = leftRightSpaced; targetX = ib.getRight() + snappingSpacing; snapIsLeft = true; }
			if (rightRight < curDistX) { curDistX = rightRight; targetX = ib.getRight(); snapIsLeft = false;  }
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


		snapLineX = Line<int>(); 
		if (targetUIX != nullptr)
		{
			snapLineX.setStart(Point<int>(targetX, jmin<int>(sb.getY(), targetUIX->getBounds().getY()) - 20));
			snapLineX.setEnd(Point<int>(targetX, jmax<int>(sb.getBottom(), targetUIX->getBounds().getBottom()) + 20));
			snapPosition.setX(snapIsLeft?targetX:targetX - bui->getWidth());
		}

		snapLineY = Line<int>();
		if (targetUIY != nullptr)
		{
			snapLineY.setStart(Point<int>(jmin<int>(sb.getX(), targetUIY->getBounds().getX()) - 20, targetY));
			snapLineY.setEnd(Point<int>(jmax<int>(sb.getRight(), targetUIY->getBounds().getRight()) + 20, targetY));
			snapPosition.setY(snapIsTop ? targetY : targetY - bui->getHeight());
		}

		targetSnapViewPosition = this->getViewPos(snapPosition);

		this->repaint();
	}

	if (updatePositionOnDragMove)
	{
		Point<float> targetPosition = (snapPosition != realP) ? targetSnapViewPosition : this->getPositionFromDrag(bui, dragSourceDetails);
		if(Desktop::getInstance().getMainMouseSource().getCurrentModifiers().isAltDown()) bui->baseItem->scalePosition(targetPosition - bui->baseItem->movePositionReference, true);
		else bui->baseItem->movePosition(targetPosition - bui->baseItem->movePositionReference, true);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemDropped(const DragAndDropTarget::SourceDetails & dragSourceDetails)
{
	BaseManagerUI<M, T, U>::itemDropped(dragSourceDetails);
	
	BaseItemMinimalUI<T> * bui = dynamic_cast<BaseItemMinimalUI<T> *>(dragSourceDetails.sourceComponent.get());

	if (bui != nullptr &&  this->itemsUI.contains((U*)bui))
	{
		Point<float> p = enableSnapping ? targetSnapViewPosition : this->getPositionFromDrag(bui, dragSourceDetails);
		if (Desktop::getInstance().getMainMouseSource().getCurrentModifiers().isAltDown()) bui->baseItem->scalePosition(p - bui->baseItem->movePositionReference, true);
		else bui->baseItem->movePosition(p - bui->baseItem->movePositionReference, true);
		bui->baseItem->addMoveToUndoManager(true);
	}

	snapLineX = Line<int>();
	snapLineY = Line<int>();
}

template<class M, class T, class U>
Point<float> BaseManagerViewUI<M, T, U>::getPositionFromDrag(Component * c, const DragAndDropTarget::SourceDetails& dragSourceDetails)
{
	Point<int> relOffset = Point<int>((int)dragSourceDetails.description.getProperty("offsetX", 0), (int)dragSourceDetails.description.getProperty("offsetY", 0));
	Point<int> realP = this->getMouseXYRelative() - (this->getLocalPoint(c, relOffset) - c->getPosition()) * 1.0f / viewZoom;
	if (centerUIAroundPosition) realP += (Point<int>(c->getWidth(), c->getHeight()) * viewZoom) / 2;
	return this->getViewPos(realP);
}

template<class M, class T, class U>
 void BaseManagerViewUI<M, T, U>::itemUIMiniModeChanged(BaseItemUI<T>* itemUI)
{
	updateViewUIPosition(dynamic_cast<U *>(itemUI));
}

 template<class M, class T, class U>
 void BaseManagerViewUI<M, T, U>::itemUIViewPositionChanged(BaseItemMinimalUI<T>* itemUI)
 {
	 updateViewUIPosition(dynamic_cast<U *>(itemUI));
 }

 template<class M, class T, class U>
 void BaseManagerViewUI<M, T, U>::askForSyncPosAndSize(BaseItemMinimalUI<T>* itemUI)
 {
	 Array<UndoableAction*> actions;
	 actions.add(itemUI->baseItem->viewUIPosition->setUndoablePoint(itemUI->baseItem->viewUIPosition->getPoint(), getViewPos(itemUI->getPosition()).toFloat(),true));
	 actions.add(itemUI->baseItem->viewUISize->setUndoablePoint(itemUI->baseItem->viewUISize->getPoint(), Point<float>	(itemUI->getWidth(), itemUI->getHeight()),true));
	 UndoMaster::getInstance()->performActions("Move / Resize "+itemUI->baseItem->niceName, actions);
 }

 template<class M, class T, class U>
 void BaseManagerViewUI<M, T, U>::newMessage(const Engine::EngineEvent& e)
 {
	frameView();
}
