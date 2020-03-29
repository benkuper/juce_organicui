#include "AutomationUI.h"
/*
  ==============================================================================

    AutomationUI.cpp
    Created: 21 Mar 2020 4:06:30pm
    Author:  bkupe

  ==============================================================================
*/

AutomationUI::AutomationUI(Automation* manager) :
    BaseManagerUI(manager->niceName, manager, false),
    paintingMode(false),
    previewMode(false)
{
    resizeOnChildBoundsChanged = false;

    animateItemOnAdd = false;
    manager->addAsyncContainerListener(this);
    
    transparentBG = true;

    addExistingItems(false);
    setSize(100, 300);
}

AutomationUI::~AutomationUI()
{
    if (!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);

    for (auto& ui : itemsUI)
    {
        if (ui != nullptr && !ui->inspectable.wasObjectDeleted())
        {
            ui->item->removeAsyncKeyListener(this);
            ui->removeKeyUIListener(this);
        }
    }
}

void AutomationUI::paint(Graphics& g)
{
    if (getWidth() == 0 || !isShowing()) return;

    if (previewMode)
    {
        Path p;
        p.startNewSubPath(Point<float>(0, getYForValue(manager->getValueAtPosition(getPosForX(0)))));
        for (int i = 1; i < getWidth(); i+=2)
        {
            p.lineTo(Point<float>(i, getYForValue(manager->getValueAtPosition(getPosForX(i)))));
        }
        g.setColour(NORMAL_COLOR);
        g.strokePath(p, PathStrokeType(1));
        return;
    }
    
    g.setColour(Colours::white.withAlpha(.04f));
    g.fillRect(getLocalBounds().withTop(getYForValue(manager->value->floatValue())));

    if (manager->items.size() > 0)
    {
        if (manager->items[0]->position->floatValue() > 0)
        {
            g.setColour(NORMAL_COLOR);
            Point<int> p = getPosInView(manager->items[0]->getPosAndValue());
            const float dashLengths[] = { 5, 5 };
            g.drawDashedLine(Line<int>(Point<int>(0, p.y), p).toFloat(), dashLengths, 2);
        }

        if (manager->items[manager->items.size()-1]->position->floatValue() < manager->length->floatValue())
        {
            g.setColour(NORMAL_COLOR);
            Point<int> p = getPosInView(manager->items[manager->items.size()-1]->getPosAndValue());
            const float dashLengths[] = { 5, 5 };
            g.drawDashedLine(Line<int>(p, Point<int>(getWidth(), p.y)).toFloat(), dashLengths, 2);
        }
    }
}

void AutomationUI::paintOverChildren(Graphics& g)
{
    if (previewMode || !isShowing()) return;

    g.setColour(GREEN_COLOR);
    g.drawEllipse(Rectangle<int>(0, 0, 6, 6).withCentre(getPosInView(manager->getPosAndValue())).toFloat(), 1.5f);

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
                    p.startNewSubPath(getPosInView(manager->recorder->keys[0]).toFloat());
                    for (int i = 1; i < numRKeys; i++)  p.lineTo(getPosInView(manager->recorder->keys[i]).toFloat());
                    
                    g.setColour(Colours::orangered);
                    g.strokePath(p, PathStrokeType(2));
                }
            }
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
            g.fillEllipse(Rectangle<int>(0, 0, 3, 3).withCentre(vpp).toFloat());
            p.lineTo(vpp.toFloat());
        }

        g.strokePath(p, PathStrokeType(1));
    }
}


void AutomationUI::resized()
{
    if (previewMode || !isShowing()) return;
    for (auto& kui : itemsUI) placeKeyUI(kui);
}

void AutomationUI::placeKeyUI(AutomationKeyUI* ui)
{
    if (ui == nullptr || !ui->isVisible()) return;

    Point<int> p = getPosInView(ui->item->getPosAndValue());
    Rectangle<int> pr = Rectangle<int>(0, 0, 20, 20).withCentre(p);
    if (ui->item->easing != nullptr) pr = pr.getUnion(getBoundsInView(ui->item->easing->getBounds(true)));
    pr.expand(5, 5);
    ui->setBounds(pr);
    ui->setValueBounds(getViewBounds(pr));

}

void AutomationUI::updateHandlesForUI(AutomationKeyUI* ui, bool checkSideItems)
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

void AutomationUI::setPreviewMode(bool value)
{
    previewMode = value;
    if (previewMode)
    {
        for (auto& kui : itemsUI) kui->setVisible(false);
    }
    else
    {
        updateItemsVisibility();
    }

    resized();
    repaint();
}

void AutomationUI::setViewRange(float start, float end)
{
    viewPosRange.setXY(start, end);
    viewLength = viewPosRange.y - viewPosRange.x;

    resized();
    repaint();
}

void AutomationUI::updateItemsVisibility()
{
    if (itemsUI.size() == 0) return;
    if (previewMode) return;

    int firstIndex = jmax(manager->items.indexOf(manager->getKeyForPosition(viewPosRange.x)), 0);
    int lastIndex = jmax(manager->items.indexOf(manager->getKeyForPosition(viewPosRange.y))+1, firstIndex);
   
    for (int i=0;i<itemsUI.size();i++)
    {
        itemsUI[i]->setVisible(i >= firstIndex && i <= lastIndex);
    }
}

void AutomationUI::addItemUIInternal(AutomationKeyUI* ui)
{
    ui->addMouseListener(this, true);
    ui->item->addAsyncKeyListener(this);
    ui->addKeyUIListener(this);
}

void AutomationUI::removeItemUIInternal(AutomationKeyUI* ui)
{
    ui->removeMouseListener(this);
    if (!ui->inspectable.wasObjectDeleted())
    {
        ui->item->removeAsyncKeyListener(this);
        ui->removeKeyUIListener(this);
    }
}

void AutomationUI::mouseDown(const MouseEvent& e)
{
    if (e.eventComponent == this)
    {
        if (e.mods.isLeftButtonDown() && e.mods.isCommandDown() && e.mods.isShiftDown())
        {
            paintingMode = true;
            paintingPoints.clear();
            paintingPoints.add(getViewPos(e.getPosition()));
        }
        else
        {
            BaseManagerUI::mouseDown(e);
        }
    }
}

void AutomationUI::mouseDrag(const MouseEvent& e)
{    
    if (AutomationKeyHandle* handle = dynamic_cast<AutomationKeyHandle*>(e.eventComponent))
    {
        AutomationKey* k = handle->key;
        int index = manager->items.indexOf(k);

        Point<float> p = getViewPos(e.getEventRelativeTo(this).getPosition());
        if (k->nextKey != nullptr) p.x = jmin(p.x, k->nextKey->position->floatValue());
        if (index > 0) p.x = jmax(p.x, manager->items[index - 1]->position->floatValue());
        k->setPosAndValue(p);
    }
    else if (e.eventComponent == this)
    {
        if (paintingMode)
        {
            Point<float> p = getViewPos(e.getPosition());
            if (p.x > paintingPoints[paintingPoints.size() - 1].x) paintingPoints.add(p);
            repaint();
        }
        else
        {
            BaseManagerUI::mouseDrag(e);
        }
    }
}

void AutomationUI::mouseUp(const MouseEvent& e)
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
        BaseManagerUI::mouseUp(e);
    }
}

void AutomationUI::mouseDoubleClick(const MouseEvent& e)
{
    if (e.eventComponent == this)
    {
        Point<float> p = getViewPos(e.getPosition());
        manager->addKey(p.x, p.y);
    }
}

Component* AutomationUI::getSelectableComponentForItemUI(AutomationKeyUI* ui)
{
    return &ui->handle;
}

Point<float> AutomationUI::getViewPos(Point<int> pos, bool relative)
{
    return Point<float>(getPosForX(pos.x, relative), getValueForY(pos.y, relative));
}

Rectangle<float> AutomationUI::getViewBounds(Rectangle<int> pos, bool relative)
{
    Rectangle<float> r = Rectangle<float>(getViewPos(pos.getBottomLeft()), getViewPos(pos.getTopRight()));
    if (relative) r.setPosition(0, 0);
    return r;
}

Point<int> AutomationUI::getPosInView(Point<float> pos, bool relative)
{
    return Point<int>(getXForPos(pos.x, relative), getYForValue(pos.y, relative));
}

Rectangle<int> AutomationUI::getBoundsInView(Rectangle<float> pos, bool relative)
{
    Rectangle<int> r = Rectangle<int>(getPosInView(pos.getTopLeft()), getPosInView(pos.getBottomRight()));
    if (relative) r.setPosition(0, 0);
    return r;
}

float AutomationUI::getPosForX(int x, bool relative)
{
    float rel = (x *1.0f / getWidth()) * viewLength;
    return relative ? rel : viewPosRange.x + rel;
}

int AutomationUI::getXForPos(float x, bool relative)
{
    return ((relative ? x : x - viewPosRange.x) / viewLength) * getWidth();
}

float AutomationUI::getValueForY(int y, bool relative)
{
    float rel = (1- y*1.0f / getHeight()) * (manager->viewValueRange->y - manager->viewValueRange->x);
    return relative ? rel : manager->viewValueRange->x + rel;
}

int AutomationUI::getYForValue(float x, bool relative)
{
    return (1 - (relative ? x : x - manager->viewValueRange->x) / (manager->viewValueRange->y - manager->viewValueRange->x)) * getHeight();
}

void AutomationUI::newMessage(const AutomationKey::AutomationKeyEvent& e)
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

void AutomationUI::newMessage(const ContainerAsyncEvent& e)
{
    if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
    {
        if (e.targetControllable == manager->value || e.targetControllable == manager->position)
        {
            repaint();
        }
        else if (e.targetControllable->parentContainer == manager->items[0] || e.targetControllable->parentContainer == manager->items[manager->items.size() - 1])
        {
            repaint();
        }
    }
}

void AutomationUI::keyEasingHandleMoved(AutomationKeyUI* ui, bool syncOtherHandle, bool isFirst)
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

