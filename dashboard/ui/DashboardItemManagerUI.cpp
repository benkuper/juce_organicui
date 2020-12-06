/*
  ==============================================================================

    DashboardItemManagerUI.cpp
    Created: 23 Apr 2017 12:33:58pm
    Author:  Ben

  ==============================================================================
*/

DashboardItemManagerUI::DashboardItemManagerUI(DashboardItemManager * manager) :
	BaseManagerViewUI("Dashboard", manager)
{

	enableSnapping = DashboardManager::getInstance()->snapping->boolValue();
	updatePositionOnDragMove = true;

	setShowAddButton(false);

	manager->addAsyncCoalescedContainerListener(this);
	DashboardManager::getInstance()->snapping->addAsyncParameterListener(this);

	File f = manager->bgImage->getFile();
	if (f.existsAsFile()) bgImage = ImageCache::getFromFile(f);


	acceptedDropTypes.add("Controllable");
	acceptedDropTypes.add("Container");

	addExistingItems();
}

DashboardItemManagerUI::~DashboardItemManagerUI()
{
	if(!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
	if(DashboardManager::getInstanceWithoutCreating() != nullptr) DashboardManager::getInstance()->snapping->removeAsyncParameterListener(this);
}

void DashboardItemManagerUI::paint(Graphics& g)
{
	BaseManagerViewUI::paint(g);

	if (manager == nullptr || inspectable.wasObjectDeleted()) return;
	float alpha = manager->bgImageAlpha->floatValue();

	if (bgImage.isNull() || alpha == 0) return;

	g.setColour(Colours::white.withAlpha(alpha));
	float scale = manager->bgImageScale->floatValue();
	Rectangle<int> r = getBoundsInView(Rectangle<float>().withSizeKeepingCentre(bgImage.getWidth()*scale, bgImage.getHeight()*scale));
	g.drawImage(bgImage, r.toFloat(), RectanglePlacement::stretchToFit, false);
	
}

void DashboardItemManagerUI::paintOverChildren(Graphics& g)
{
	BaseManagerViewUI::paintOverChildren(g);

	if (manager == nullptr || inspectable.wasObjectDeleted()) return;
	if (manager->canvasSize->enabled && manager->canvasSize->x > 0 && manager->canvasSize->y > 0)
	{
		Point<float> p = manager->canvasSize->getPoint();
		Rectangle<int> canvasR = getBoundsInView(Rectangle<float>().withSizeKeepingCentre(p.x, p.y));
		Path path;
		path.addRectangle(getLocalBounds());
		path.setUsingNonZeroWinding(false);
		path.addRectangle(canvasR);

		g.setColour(Colours::black.withAlpha(.5f));
		g.fillPath(path);
	}
}

bool DashboardItemManagerUI::isInterestedInDragSource(const SourceDetails & dragSourceDetails)
{
	return true;
}

void DashboardItemManagerUI::itemDropped(const SourceDetails & details)
{
	BaseManagerViewUI::itemDropped(details);

	if (details.sourceComponent->getParentComponent() == this) return;

	InspectableContentComponent * icc = dynamic_cast<InspectableContentComponent *>(details.sourceComponent.get());
	if (icc != nullptr && icc->inspectable != nullptr)
	{
		BaseItem * bi = dynamic_cast<BaseItem *>(icc->inspectable.get());
		if (bi != nullptr)
		{
			manager->addItem(bi->createDashboardItem(), getViewMousePosition().toFloat());
		}
		return;
	}
	
	ControllableEditor* e = dynamic_cast<ControllableEditor*>(details.sourceComponent.get());
	if (e != nullptr)
	{
		manager->addItem(e->controllable->createDashboardItem(), getViewMousePosition().toFloat());
		return;
	}

	GenericControllableContainerEditor* ge = dynamic_cast<GenericControllableContainerEditor*>(details.sourceComponent.get());
	if (ge != nullptr)
	{
		manager->addItem(ge->container->createDashboardItem(), getViewMousePosition().toFloat());
		return;
	}
}

void DashboardItemManagerUI::showMenuAndAddItem(bool fromAddButton, Point<int> mousePos)
{
	if (!DashboardManager::getInstance()->editMode->boolValue()) return;
	BaseManagerViewUI::showMenuAndAddItem(fromAddButton, mousePos);

}

BaseItemMinimalUI<DashboardItem> * DashboardItemManagerUI::createUIForItem(DashboardItem * item)
{
	return item->createUI();
}

void DashboardItemManagerUI::newMessage(const ContainerAsyncEvent& e)
{
	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
		if (e.targetControllable.wasObjectDeleted()) return;
		
		if (e.targetControllable == manager->bgImage)
		{
			File f = manager->bgImage->getFile();
			if (f.existsAsFile()) bgImage = ImageCache::getFromFile(f);
			else bgImage = Image();
			repaint();
		}
		else if (e.targetControllable == manager->bgImageAlpha || e.targetControllable == manager->bgImageScale || e.targetControllable == manager->canvasSize)
		{
			repaint();
		}

		break;
        default:
            break;
	}
}

void DashboardItemManagerUI::newMessage(const Parameter::ParameterEvent& e)
{
	if (e.parameter == DashboardManager::getInstance()->snapping)
	{
		enableSnapping = DashboardManager::getInstance()->snapping->boolValue();
	}
}
