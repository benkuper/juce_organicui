/*
  ==============================================================================

	BaseManagerViewUI.h
	Created: 23 Apr 2017 2:48:02pm
	Author:  Ben

  ==============================================================================
*/

#ifndef BASEMANAGERVIEWUI_H_INCLUDED
#define BASEMANAGERVIEWUI_H_INCLUDED


template<class M, class T, class U>
class BaseManagerViewUI :
	public BaseManagerUI<M, T, U>
{
public:
	BaseManagerViewUI<M, T, U>(const String &contentName, M * _manager);
	virtual ~BaseManagerViewUI();

	bool canNavigate;
	bool canZoom;

	Point<int> viewOffset; //in pixels, viewOffset of 0 means zeroPos is at the center of the window
						   //interaction
	Point<int> initViewOffset;

	const int defaultCheckerSize = 32;
	float viewZoom;

	virtual void mouseDown(const MouseEvent &e) override;
	virtual void mouseDrag(const MouseEvent &e) override;
	virtual void mouseUp(const MouseEvent &e) override;
	void mouseWheelMove(const MouseEvent &e, const MouseWheelDetails &d) override;
	virtual bool keyPressed(const KeyPress &e) override;

	virtual void paint(Graphics &g) override;
	virtual void paintBackground(Graphics &g);

	virtual void resized() override;

	virtual void updateViewUIPosition(U * se);

	virtual void addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos) override;

	Point<int> getSize();
	Point<int> getViewMousePosition();
	Point<int> getViewPos(const Point<int> &originalPos);
	float getUnitSize();
	Point<int> getViewCenter();
	Point<float> getPosInView(const Point<float> &viewPos);
	Rectangle<float> getBoundsInView(const Rectangle<float> &r);
	Point<float> getItemsCenter();

	virtual void homeView();
	virtual void frameView();

	virtual void setViewZoom(float newZoom);

	virtual void addItemUIInternal(U * se) override; 

	virtual void itemUIGrabbed(BaseItemUI<T> * se) override;
	virtual void itemUIMiniModeChanged(BaseItemUI<T> * se) override;

	virtual void endLoadFile() override;
};



template<class M, class T, class U>
BaseManagerViewUI<M, T, U>::BaseManagerViewUI(const String & contentName, M * _manager) :
	BaseManagerUI<M, T, U>(contentName, _manager, false),
	canNavigate(true),
	canZoom(true),
    viewZoom(1)
{
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
	if (canZoom && e.originalComponent == this) setViewZoom(jlimit<float>(.1f, 10, viewZoom + d.deltaY));
	
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
	paintBackground(g);

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
	int checkerSize = getUnitSize();

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
void BaseManagerViewUI<M, T, U>::updateViewUIPosition(U * se)
{	
	if (se == nullptr) return;
	Point<int> pe = se->item->viewUIPosition->getPoint().toInt() * viewZoom;
	
	pe += getSize()/2; //position at center of window
	pe += viewOffset;
	pe -= se->getLocalBounds().getCentre();
	se->setTopLeftPosition(pe.x, pe.y);
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos)
{
	this->manager->addItem(getViewPos(mouseDownPos).toFloat());
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
	return originalPos - getSize() / 2 - viewOffset;
}

template<class M, class T, class U>
float BaseManagerViewUI<M, T, U>::getUnitSize()
{
	return defaultCheckerSize*viewZoom;
}

template<class M, class T, class U>
inline Point<int> BaseManagerViewUI<M, T, U>::getViewCenter()
{
	return viewOffset + (getSize() / 2);
}

template<class M, class T, class U>
Point<float> BaseManagerViewUI<M, T, U>::getPosInView(const Point<float>& viewPos)
{
	return (viewPos*getUnitSize()) + getViewCenter().toFloat();
}

template<class M, class T, class U>
Rectangle<float> BaseManagerViewUI<M, T, U>::getBoundsInView(const Rectangle<float>& r)
{
	return r.withPosition(getPosInView(r.getPosition())).withSize(r.getWidth()*getUnitSize(),r.getHeight()*getUnitSize());
}

template<class M, class T, class U>
Point<float> BaseManagerViewUI<M, T, U>::getItemsCenter()
{
	if (this->itemsUI.size() == 0) return Point<float>(0, 0);

	Point<float> average;
	for (auto &se : this->itemsUI)
	{
		average += se->item->viewUIPosition->getPoint();
	}
	average /= this->itemsUI.size();
	return average;
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
	viewZoom = value;
	for (auto &tui : this->itemsUI) tui->setViewZoom(value);

	this->resized();
	this->repaint();
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::addItemUIInternal(U * se)
{
	se->setViewZoom(viewZoom);
	updateViewUIPosition(se); 
}


template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::itemUIGrabbed(BaseItemUI<T> * se)
{
	updateViewUIPosition(static_cast<U *>(se));
}

template<class M, class T, class U>
inline void BaseManagerViewUI<M, T, U>::itemUIMiniModeChanged(BaseItemUI<T>* se)
{
	updateViewUIPosition(static_cast<U *>(se));
}

template<class M, class T, class U>
void BaseManagerViewUI<M, T, U>::endLoadFile()
{
	frameView();
}




#endif  // BASEMANAGERVIEWUI_H_INCLUDED
