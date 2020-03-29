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
    paintingMode(false)
{
    animateItemOnAdd = false;
    manager->addAsyncContainerListener(this);

    bgColor = BG_COLOR.brighter(.1f);

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

void AutomationUI::paintOverChildren(Graphics& g)
{

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
    for (auto& kui : itemsUI) placeKeyUI(kui);
}

void AutomationUI::placeKeyUI(AutomationKeyUI* ui)
{
    if (ui == nullptr) return;

    Point<int> p = getPosInView(ui->item->getPosAndValue());
    Rectangle<int> pr = Rectangle<int>(0, 0, 20, 20).withCentre(p);
    if (ui->item->easing != nullptr) pr = pr.getUnion(getBoundsInView(ui->item->easing->getBounds(true)));
    pr.expand(5, 5);
    ui->setBounds(pr);
    ui->setValueBounds(getViewBounds(pr));

   // DBG("place key ui " << itemsUI.indexOf(ui) << " : " << ui->getBounds().toString()  << " / " << (int)ui->isShowing());
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


    bool prevSelected = index > 0 && itemsUI[index - 1] != nullptr && itemsUI[index - 1]->item->isThisOrChildSelected();
    bool nextSelected = index < itemsUI.size() && itemsUI[index + 1] != nullptr && itemsUI[index + 1]->item->isThisOrChildSelected();

    ui->setShowEasingHandles(prevSelected, nextSelected);

}

void AutomationUI::setViewRange(float start, float end)
{
    viewPosRange.setXY(start, end);
    viewLength = viewPosRange.y - viewPosRange.x;
    resized();
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
        paintingMode = e.mods.isLeftButtonDown() && e.mods.isCommandDown() && e.mods.isShiftDown();
        paintingPoints.clear();
        paintingPoints.add(getViewPos(e.getPosition()));
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
            paintingPoints.add(getViewPos(e.getPosition()));
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
        paintingPoints.clear();
        paintingMode = false;
    }
}

void AutomationUI::mouseDoubleClick(const MouseEvent& e)
{
    if (e.eventComponent == this)
    {
        Point<float> p = getViewPos(e.getPosition());
        manager->addKey(p.x, p.y);
    }
    /*
    else if (EasingUI* eui = dynamic_cast<EasingUI*>(e.eventComponent))
    {
        float p = eui->easing->getClosestPointForPos(getViewPos(e.getPosition()));
        AutomationKey* k = manager->createItem();
        k->setPosAndValue(p);
        var params(new DynamicObject());
        AutomationKeyUI* kui = dynamic_cast<AutomationKeyUI*>(eui->getParentComponent());
        params.getDynamicObject()->setProperty("index", itemsUI.indexOf(kui) + 1);
        manager->addItem(k, params);
    }
    */
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

