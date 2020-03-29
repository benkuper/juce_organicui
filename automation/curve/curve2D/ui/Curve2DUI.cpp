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

    addExistingItems(false);
    setSize(100, 300);
}

Curve2DUI::~Curve2DUI()
{
    if(!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);

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
            g.fillEllipse(Rectangle<int>(0, 0, 3, 3).withCentre(vpp).toFloat());
            p.lineTo(vpp.toFloat());
        }

        g.strokePath(p, PathStrokeType(1));
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
        Array<float> points;
        for (auto& pp : paintingPoints) points.add(pp.x, pp.y);
        float* result;
        unsigned int resultNum = 0;
        unsigned int * origIndex;
        unsigned int* corners = nullptr;
        unsigned int cornersLength = 0;
        unsigned int* cornerIndex = nullptr;
        unsigned int cornerIndexLength = 0;

        int detectResult = curve_fit_corners_detect_fl(points.getRawDataPointer(), points.size(), 2, 0, .02f, 20, 30, &corners, &cornersLength);
        if (cornersLength == 0) corners = nullptr;

        int fitResult = curve_fit_cubic_to_points_fl(points.getRawDataPointer(), points.size(), 2, .1f, CURVE_FIT_CALC_HIGH_QUALIY, corners, cornersLength, &result, &resultNum, &origIndex, &cornerIndex, &cornerIndexLength);
        
        int numPoints = ((int)resultNum);

        Array<Curve2DKey*> keys;

        Curve2DKey* prevKey = nullptr;
        CubicEasing2D * prevEasing = nullptr;

        for (int i = 0; i < numPoints; i++)
        {
            int index = i * 6;
            Point<float> h1(result[index+0], result[index+1]);
            Point<float> rp(result[index+2], result[index+3]);
            Point<float> h2(result[index+4], result[index+5]);

           
            if (prevEasing != nullptr && h1.getDistanceFromOrigin() < 100)
            {
                prevEasing->anchor2->setPoint(h1 - rp);
            }

            if (rp.getDistanceFromOrigin() > 100)
            {
                break;
            }

            Curve2DKey * k = new Curve2DKey();
            k->position->setPoint(rp);
           
            k->easingType->setValueWithData(Easing2D::BEZIER);
            CubicEasing2D* ce = (CubicEasing2D*)k->easing.get();
            if (h2.getDistanceFromOrigin() < 100) ce->anchor1->setPoint(h2 - rp);
            
            keys.add(k);

            prevEasing = ce;
        }

        delete result;
        delete origIndex;
        delete corners;
        delete cornerIndex;

        manager->addItems(keys);

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

