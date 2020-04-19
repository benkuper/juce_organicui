#include "Curve2DUI.h"
/*
  ==============================================================================

    Curve2DUI.cpp
    Created: 21 Mar 2020 4:06:30pm
    Author:  bkupe

  ==============================================================================
*/




Curve2DUI::Curve2DUI(Curve2D* manager) :
    BaseManagerViewUI(manager->niceName, manager),
    paintingMode(false)
{
    useCheckersAsUnits = true;
    minZoom = .1f;
    maxZoom = 10;

    animateItemOnAdd = false;
    manager->addAsyncContainerListener(this);
    if (manager->recorder != nullptr) manager->recorder->addAsyncCoalescedRecorderListener(this);

    addExistingItems(false);
    setSize(100, 300);
}

Curve2DUI::~Curve2DUI()
{
    if (!inspectable.wasObjectDeleted())
    {
        manager->removeAsyncContainerListener(this);
        if (manager->recorder != nullptr) manager->recorder->removeAsyncRecorderListener(this);
    }

    for (auto& ui : itemsUI)
    {
        if (ui != nullptr && !ui->inspectable.wasObjectDeleted())
        {
            ui->item->removeAsyncKeyListener(this);
            ui->removeKeyUIListener(this);
        }
    }
}

void Curve2DUI::paintOverChildren(Graphics& g)
{
    g.setColour(GREEN_COLOR);
    g.drawEllipse(Rectangle<int>(0, 0, 8, 8).withCentre(getPosInView(manager->value->getPoint())).toFloat(), 2);

    if (paintingMode && paintingPoints.size() > 0)
    {
        g.setColour(YELLOW_COLOR);
        Path p;
        p.startNewSubPath(getPosInView(paintingPoints[0]).toFloat());
        for (auto& pp : paintingPoints)
        {
            Point<int> vpp = getPosInView(pp);
            g.fillEllipse(Rectangle<int>(0, 0, 2, 2).withCentre(vpp).toFloat());
            p.lineTo(vpp.toFloat());
        }

        g.setColour(YELLOW_COLOR.withAlpha(.5f));
        g.strokePath(p, PathStrokeType(1));
    }

    //recorder
    if (manager->recorder != nullptr)
    {
        if (manager->recorder->isRecording->boolValue())
        {
            int numRKeys = manager->recorder->keys.size();
            if (numRKeys > 0)
            {
                if (numRKeys >= 2)
                {
                    Path p;
                    Point<float> k0(manager->recorder->keys[0].value[0], manager->recorder->keys[0].value[1]);
                    p.startNewSubPath(getPosInView(k0).toFloat());
                    for (int i = 1; i < numRKeys; i++)
                    {
                        Point<float> ki(manager->recorder->keys[i].value[0], manager->recorder->keys[i].value[1]);
                        p.lineTo(getPosInView(ki).toFloat());
                    }
                    g.setColour(Colours::orangered);
                    g.strokePath(p, PathStrokeType(2));
                }
            }
        }
    }
}

void Curve2DUI::updateViewUIPosition(Curve2DKeyUI* ui)
{
    Point<int> p = getPosInView(ui->item->position->getPoint());
    Rectangle<int> pr = Rectangle<int>(0, 0, 20, 20).withCentre(p);
    if (ui->item->easing != nullptr) pr = pr.getUnion(getBoundsInView(ui->item->easing->getBounds(true)));
    pr.expand(5, 5);
    ui->setBounds(pr);
    ui->setValueBounds(getViewBounds(pr));
}

void Curve2DUI::updateHandlesForUI(Curve2DKeyUI* ui, bool checkSideItems)
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
        Curve2DKey* prevItem = itemsUI[index - 1]->item;
        prevSelected = prevItem->isThisOrChildSelected() && !prevItem->isSelected; //we only want to show if easing is selected only easing
    }
    bool nextSelected = index < itemsUI.size() && itemsUI[index + 1] != nullptr && itemsUI[index + 1]->item->isThisOrChildSelected();

    ui->setShowEasingHandles(prevSelected, nextSelected);

}

void Curve2DUI::addItemUIInternal(Curve2DKeyUI* ui)
{
    ui->addMouseListener(this, true);
    ui->item->addAsyncKeyListener(this);
    ui->addKeyUIListener(this);
}

void Curve2DUI::removeItemUIInternal(Curve2DKeyUI* ui)
{
    ui->removeMouseListener(this);
    if (!ui->inspectable.wasObjectDeleted())
    {
        ui->item->removeAsyncKeyListener(this);
        ui->removeKeyUIListener(this);
    }
}

void Curve2DUI::mouseDown(const MouseEvent& e)
{
    if (e.eventComponent == this && e.mods.isLeftButtonDown() && e.mods.isCommandDown() && e.mods.isShiftDown())
    {
        paintingMode = true;
        paintingPoints.clear();
        paintingPoints.add(getViewPos(e.getPosition()));
    }
    else
    {
        BaseManagerViewUI::mouseDown(e);
    }
}

void Curve2DUI::mouseDrag(const MouseEvent& e)
{
    if (Curve2DKeyHandle* handle = dynamic_cast<Curve2DKeyHandle*>(e.eventComponent))
    {
        handle->key->position->setPoint(getViewMousePosition());
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
            BaseManagerViewUI::mouseDrag(e);
        }
    }
}

void Curve2DUI::mouseUp(const MouseEvent& e)
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
        BaseManagerViewUI::mouseUp(e);
    }
}

void Curve2DUI::mouseDoubleClick(const MouseEvent& e)
{
    if (e.eventComponent == this)
    {
        Point<float> p = getViewMousePosition();
        Curve2DKey* k = manager->createItem();
        k->position->setPoint(p);
        manager->addItem(k);
    }
    else if (Easing2DUI* eui = dynamic_cast<Easing2DUI*>(e.eventComponent))
    {
        Point<float> p = eui->easing->getClosestPointForPos(getViewMousePosition());
        Curve2DKey* k = manager->createItem();
        k->position->setPoint(p);
        var params(new DynamicObject());
        Curve2DKeyUI* kui = dynamic_cast<Curve2DKeyUI*>(eui->getParentComponent());
        params.getDynamicObject()->setProperty("index", itemsUI.indexOf(kui)+1);
        manager->addItem(k, params);
    }
}

Component* Curve2DUI::getSelectableComponentForItemUI(Curve2DKeyUI* ui)
{
    return &ui->handle;
}

void Curve2DUI::newMessage(const Curve2DKey::Curve2DKeyEvent& e)
{
    switch (e.type)
    {
    case Curve2DKey::Curve2DKeyEvent::KEY_UPDATED:
    {
        updateViewUIPosition(getUIForItem(e.key));
    }
    break;

    case Curve2DKey::Curve2DKeyEvent::SELECTION_CHANGED:
    {
        updateHandlesForUI(getUIForItem(e.key), true);
    }
    break;
    }
}

void Curve2DUI::newMessage(const ContainerAsyncEvent& e)
{
    if (e.type == ContainerAsyncEvent::ControllableFeedbackUpdate)
    {
        if (e.targetControllable == manager->value)
        {
            repaint();
        }
    }
}

void Curve2DUI::newMessage(const AutomationRecorder::RecorderEvent& e)
{
    if (e.type == AutomationRecorder::RecorderEvent::RECORDER_UPDATED) repaint();
}

void Curve2DUI::keyEasingHandleMoved(Curve2DKeyUI* ui, bool syncOtherHandle, bool isFirst)
{
    if (syncOtherHandle)
    {
        int index = itemsUI.indexOf(ui);
        if (isFirst)
        {
            if (index > 0)
            {
                if (itemsUI[index - 1]->item->easingType->getValueDataAsEnum<Easing2D::Type>() == Easing2D::BEZIER)
                {
                    if (CubicEasing2D* ce = dynamic_cast<CubicEasing2D*>(itemsUI[index - 1]->item->easing.get()))
                    {
                        CubicEasing2D* e = dynamic_cast<CubicEasing2D*>(ui->item->easing.get());
                        ce->anchor2->setPoint(-e->anchor1->getPoint());
                    }
                }
            }
        }
        else
        {
            if (index < itemsUI.size() - 2)
            {
                if (itemsUI[index + 1]->item->easingType->getValueDataAsEnum<Easing2D::Type>() == Easing2D::BEZIER)
                {
                    if (CubicEasing2D* ce = dynamic_cast<CubicEasing2D*>(itemsUI[index + 1]->item->easing.get()))
                    {
                        CubicEasing2D* e = dynamic_cast<CubicEasing2D*>(ui->item->easing.get());
                        ce->anchor1->setPoint(-e->anchor2->getPoint());
                    }
                }
            }
        }
    }
}

