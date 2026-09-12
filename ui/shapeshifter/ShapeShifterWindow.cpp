/*
  ==============================================================================

    ShapeShifterWindow.cpp
    Created: 2 May 2016 4:10:48pm
    Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

ShapeShifterWindow::ShapeShifterWindow(ShapeShifterPanel* _panel, juce::Rectangle<int> bounds) :
	ResizableWindow(_panel->currentContent != nullptr ? _panel->currentContent->contentName : "", true),
	panel(_panel),
	dragMode(NONE),
	dragStarted(false)
{
	setTopLeftPosition(bounds.getTopLeft());
	panel->setBounds(bounds);

	panel->setPreferredWidth(getWidth());
	panel->setPreferredHeight(getHeight());
    
	setAlwaysOnTop(true);
	
	//DBG("window -> addShapeShifterListener " << panel->header.tabs[0]->content->contentName);
	panel->addShapeShifterPanelListener(this); //is it necessary ?

	pinBT.reset(AssetManager::getInstance()->getToggleBTImage(ImageCache::getFromMemory(OrganicUIBinaryData::pin_png, OrganicUIBinaryData::pin_pngSize)));
	pinBT->setToggleState(isAlwaysOnTop(), dontSendNotification);
	pinBT->addListener(this);
	panel->addAndMakeVisible(pinBT.get());

	setContentNonOwned(panel, true);

	setBackgroundColour(BG_COLOR.darker(.1f).withAlpha(.3f));

	setResizable(true, true);
	setDraggable(false);

	setVisible(true);
	toFront(true);

	addMouseListener(this,true);

}

ShapeShifterWindow::~ShapeShifterWindow()
{
	removeMouseListener(this);
	clear();

}

void ShapeShifterWindow::resized()
{
	ResizableWindow::resized();

	if (panel == nullptr) return;


	panel->setPreferredWidth(getWidth());
	panel->setPreferredHeight(getHeight());
	pinBT->setBounds(panel->getLocalBounds().removeFromTop(20).removeFromRight(20).reduced(2));
}

void ShapeShifterWindow::beginDrag(const MouseEvent& e, DragMode mode)
{
	if (ShapeShifterManager::getInstance()->lockMode) return;
	dragMode = mode;
	dragStarted = true;
	dragger.startDraggingComponent(this, e);
	mouseDrag(e);
}

void ShapeShifterWindow::mouseDown(const MouseEvent & e)
{
	if (!ShapeShifterManager::getInstance()->lockMode &&
		(e.eventComponent == &panel->header ||
		 (dynamic_cast<ShapeShifterPanelTab *>(e.eventComponent) != nullptr && panel->contents.size() == 1)))
	{
		dragMode = e.eventComponent == &panel->header ? PANEL : TAB;
		dragStarted = false;
		dragger.startDraggingComponent(this, e);
	}else
	{
		dragMode = NONE;
	}

}

void ShapeShifterWindow::mouseDrag(const MouseEvent & e)
{
	if (dragMode == NONE) return;
	if (!dragStarted && e.getDistanceFromDragStart() < 8) return;
	dragStarted = true;
	panel->setTransparentBackground(true);
	setAlpha(.82f);
	dragger.dragComponent(this, e, 0);
	ShapeShifterManager::getInstance()->checkCandidateTargetForPanel(panel, e.getScreenPosition());
}

void ShapeShifterWindow::mouseUp(const MouseEvent & e)
{
	if (dragMode == NONE) return;
	const bool wasDragging = dragStarted;
	dragMode = NONE;
	dragStarted = false;
	panel->setTransparentBackground(false);
	setAlpha(1.0f);
	if (!wasDragging) return;

	// Docking can close this floating window, so perform it after the mouse callback returns.
	juce::WeakReference<ShapeShifterPanel> draggedPanel(panel);
	const auto screenPoint = e.getScreenPosition();
	ShapeShifterManager::getInstance()->setCurrentCandidatePanel(nullptr);
	MessageManager::callAsync([draggedPanel, screenPoint]()
	{
		if (draggedPanel.wasObjectDeleted()) return;
		auto* manager = ShapeShifterManager::getInstance();
		manager->checkCandidateTargetForPanel(draggedPanel.get(), screenPoint);
		manager->checkDropOnCandidateTarget(draggedPanel);
	});
}

void ShapeShifterWindow::clear()
{
	if (panel != nullptr)
	{
		panel->removeShapeShifterPanelListener(this);
		panel = nullptr;
	}
}

void ShapeShifterWindow::userTriedToCloseWindow()
{
	ShapeShifterManager::getInstance()->closePanelWindow(this,true);
}

var ShapeShifterWindow::getCurrentLayout()
{
	var data(new DynamicObject());
	data.getDynamicObject()->setProperty("panel", panel->getCurrentLayout());
	data.getDynamicObject()->setProperty("x", getBounds().getPosition().x);
	data.getDynamicObject()->setProperty("y", getBounds().getPosition().y);
	data.getDynamicObject()->setProperty("width", getWidth());
	data.getDynamicObject()->setProperty("height", getHeight());
	return data;
}

void ShapeShifterWindow::buttonClicked(Button* b)
{
	if (b == pinBT.get())
	{
		setAlwaysOnTop(!isAlwaysOnTop());
		pinBT->setToggleState(isAlwaysOnTop(), dontSendNotification);
	}
}

void ShapeShifterWindow::panelEmptied(ShapeShifterPanel *)
{
	ShapeShifterManager::getInstance()->closePanelWindow(this, false);
}
