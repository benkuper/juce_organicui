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
	BaseManagerViewUI<M, T, U>(const String &contentName, M * _manager);
	virtual ~BaseManagerViewUI();

	bool canNavigate;

	//
	bool centerUIAroundPosition;
	bool updatePositionOnDragMove;

	//Zoom
	bool canZoom;
	bool zoomAffectsItemSize;
	float viewZoom;
	float minZoom;
	float maxZoom;

	Point<int> viewOffset; //in pixels, viewOffset of 0 means zeroPos is at the center of the window
						   //interaction
	Point<int> initViewOffset;

	const int defaultCheckerSize = 64;

	virtual void mouseDown(const MouseEvent &e) override;
	virtual void mouseDrag(const MouseEvent &e) override;
	virtual void mouseUp(const MouseEvent &e) override;
	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& d) override;
	void mouseMagnify(const MouseEvent &e, float scaleFactor) override;
	virtual bool keyPressed(const KeyPress &e) override;

	virtual void paint(Graphics &g) override;
	virtual void paintBackground(Graphics &g);

	virtual void resized() override;

	virtual void updateViewUIPosition(U * se);
	virtual void updateComponentViewPosition(Component* c, Point<float> position);

	virtual void updateItemsVisibility() override;

	virtual void addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos) override;
	virtual void addItemFromMenu(T * item, bool isFromAddButton, Point<int> mouseDownPos) override;

	Point<int> getSize();
	Point<int> getViewMousePosition();
	Point<int> getViewPos(const Point<int> &originalPos);
	Point<int> getViewCenter();
	Point<float> getPosInView(const Point<float> &viewPos);
	juce::Rectangle<float> getBoundsInView(const juce::Rectangle<float> &r);
	Point<float> getItemsCenter();

	virtual void homeView();
	virtual void frameView();

	virtual void setViewZoom(float newZoom);

	virtual void addItemUIInternal(U * se) override; 

	//virtual void itemUIGrabbed(BaseItemMinimalUI<T> * se) override;

	void itemDragMove(const DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const DragAndDropTarget::SourceDetails &dragSourceDetails) override;

	virtual Point<int> getPositionFromDrag(Component * c, const DragAndDropTarget::SourceDetails& dragSourceDetails);

	virtual void itemUIMiniModeChanged(BaseItemUI<T> * itemUI) override;
	virtual void itemUIViewPositionChanged(BaseItemMinimalUI<T> * itemUI) override;


	virtual void endLoadFile() override;
};



template<class M, class T, class U>
BaseManagerViewUI<M, T, U>::BaseManagerViewUI(const String & contentName, M * _manager) :
	BaseManagerUI<M, T, U>(contentName, _manager, false),
	canNavigate(true),
	centerUIAroundPosition(false),
	updatePositionOnDragMove(false),
	canZoom(true),
	zoomAffectsItemSize(true),
	viewZoom(1),
	minZoom(.4f),
	maxZoom(1)

{
    this->defaultLayout = this->FREE;

	this->resizeOnChildBoundsChanged = false;
	this->bgColor = BG_COLOR.darker(.3f);
}


template<class M, class T, class U>
BaseManagerViewUI<M, T, U>::~BaseManagerViewUI()
{
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
			 Point<int> curMousePos = getViewMousePosition();
			 
			 setViewZoom(viewZoom + d.deltaY);
			 
			 Point<int> newMousePos = getViewMousePosition();
			 viewOffset += (newMousePos - curMousePos) * viewZoom;
			 updateItemsVisibility();
			 this->resized();
		 }
	 }
	 else
	 {
		 float sensitivity = 500;
		 viewOffset += Point<int>(d.deltaX*sensitivity, d.deltaY*sensitivity);
		 updateItemsVisibility(); 
		 this->resized();
		 this->repaint();
	 }
	
}

 template<class M, class T, class U>
 void BaseManagerViewUI<M, T, U>::mouseMagnify(const MouseEvent& e, float scaleFactor)
 {
	 if (canZoom)
	 {
		 Point<int> curMousePos = getViewMousePosition();

		 setViewZoom(viewZoom + scaleFactor);

		 Point<int> newMousePos = getViewMousePosition();
		 viewOffset += (newMousePos - curMousePos) * viewZoom;
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
	int checkerSize = defaultCheckerSize * viewZoom;

	int checkerTX = -checkerSize * 2 + ((this->getWidth() / 2 + viewOffset.x) % (checkerSize * 2));
	int checkerTY = -checkerSize * 2 + ((this->getHeight() / 2 + viewOffset.y) % (checkerSize * 2));
	juce::Rectangle<int> checkerBounds(checkerTX, checkerTY, this->getWidth() + checkerSize * 4, this->getHeight() + checkerSize * 4);
	g.fillCheckerBoard(checkerBounds.toFloat(), checkerSize, checkerSize, BG_COLOR.darker(.3f), BG_COLOR.darker(.2f));

	g.setColour(BG_COLOR.darker(.05f));
	Point<int> center = getSize() / 2;
	center += viewOffset;
	g.drawLine(center.x, 0, center.x, this->getHeight(), 2);
	g.drawLine(0, center.y, this->getWidth(), center.y, 2);

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
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateViewUIPosition(U * itemUI)
{	
	if (itemUI == nullptr) return;
	updateComponentViewPosition((Component *)itemUI, itemUI->item->viewUIPosition->getPoint());
	
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateComponentViewPosition(Component* c, Point<float> position)
{
	if (c == nullptr) return;

	Point<int> p = getPosInView(position).toInt();

	if (centerUIAroundPosition) p -= (Point<int>(c->getWidth(), c->getHeight()) * viewZoom) / 2;
	c->setTopLeftPosition(p.x, p.y);

	if (zoomAffectsItemSize)
	{
		AffineTransform t = AffineTransform().scaled(viewZoom, viewZoom, p.x, p.y);
		c->setTransform(t);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::updateItemsVisibility()
{
	//BaseManagerUI::updateItemsVisibility();
	juce::Rectangle<int> r = this->getLocalBounds();
	for (auto &iui : this->itemsUI)
	{
		juce::Rectangle<int> ir = iui->getBounds().getIntersection(r);
		bool isInsideInspectorBounds = !ir.isEmpty();
		iui->setVisible(isInsideInspectorBounds);
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos)
{
	this->manager->addItem(getViewPos(mouseDownPos).toFloat());
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemFromMenu(T * item, bool isFromAddButton, Point<int> mouseDownPos)
{
	this->manager->addItem(item, getViewPos(mouseDownPos).toFloat());
}

template<class M, class T, class U>
Point<int> BaseManagerViewUI<M, T, U>::getSize()
{
	return Point<int>(this->getWidth(), this->getHeight());
}

template<class M, class T, class U>
Point<int> BaseManagerViewUI<M, T, U>::getViewMousePosition()
{
	return getViewPos(this->getMouseXYRelative());
}

template<class M, class T, class U>
Point<int> BaseManagerViewUI<M, T, U>::getViewPos(const Point<int>& originalPos)
{
	return (originalPos - getViewCenter()) / viewZoom;
}

template<class M, class T, class U>
 Point<int> BaseManagerViewUI<M, T, U>::getViewCenter()
{
	return viewOffset + (getSize() / 2);
}

template<class M, class T, class U>
Point<float> BaseManagerViewUI<M, T, U>::getPosInView(const Point<float>& viewPos)
{
	return viewPos * viewZoom + getViewCenter().toFloat();
}

template<class M, class T, class U>
 juce::Rectangle<float> BaseManagerViewUI<M, T, U>::getBoundsInView(const juce::Rectangle<float>& r)
{
	 return juce:: Rectangle<float>().withPosition(getPosInView(r.getPosition())).withSize(r.getWidth()*viewZoom, r.getHeight()*viewZoom);
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
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::frameView()
{
	viewOffset = -getItemsCenter().toInt();
	this->resized();
	this->repaint();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::setViewZoom(float value)
{
	if (viewZoom == value) return;
	viewZoom = jlimit<float>(minZoom, maxZoom, value);
	for (auto &tui : this->itemsUI) tui->setViewZoom(viewZoom);

	this->resized();
	this->repaint();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemUIInternal(U * se)
{
	se->setViewZoom(viewZoom);
	se->resized();
	updateViewUIPosition(se);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemDragMove(const DragAndDropTarget::SourceDetails& dragSourceDetails)
{
	BaseManagerUI<M, T, U>::itemDragMove(dragSourceDetails);

	if (updatePositionOnDragMove)
	{
		BaseItemMinimalUI<T>* bui = dynamic_cast<BaseItemMinimalUI<T>*>(dragSourceDetails.sourceComponent.get());

		if (bui != nullptr)
		{
			Point<int> p = this->getPositionFromDrag(bui, dragSourceDetails);
			bui->item->viewUIPosition->setUndoablePoint(bui->item->viewUIPosition->getPoint(), p.toFloat());
		}
	}
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemDropped(const DragAndDropTarget::SourceDetails & dragSourceDetails)
{
	BaseManagerUI<M, T, U>::itemDropped(dragSourceDetails);
	
	BaseItemMinimalUI<T> * bui = dynamic_cast<BaseItemMinimalUI<T> *>(dragSourceDetails.sourceComponent.get());

	if (bui != nullptr &&  this->itemsUI.contains((U*)bui))
	{
		Point<int> p = this->getPositionFromDrag(bui, dragSourceDetails);
		bui->item->viewUIPosition->setUndoablePoint(bui->item->viewUIPosition->getPoint(), p.toFloat());
	}
}

template<class M, class T, class U>
Point<int> BaseManagerViewUI<M, T, U>::getPositionFromDrag(Component * c, const DragAndDropTarget::SourceDetails& dragSourceDetails)
{
	Point<int> realP = this->getMouseXYRelative() - Point<int>((int)dragSourceDetails.description.getProperty("offsetX", 0), (int)dragSourceDetails.description.getProperty("offsetY", 0));
	if (c != nullptr && centerUIAroundPosition) realP += (Point<int>(c->getWidth(), c->getHeight()) * viewZoom) / 2;
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
void BaseManagerViewUI<M, T, U>::endLoadFile()
{
	frameView();
}
