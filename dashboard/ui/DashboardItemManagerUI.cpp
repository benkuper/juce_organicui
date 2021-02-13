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


	manager->addAsyncCoalescedContainerListener(this);
	DashboardManager::getInstance()->snapping->addAsyncParameterListener(this);

	File f = manager->bgImage->getFile();
	if (f.existsAsFile()) bgImage = ImageCache::getFromFile(f);


	acceptedDropTypes.add("Controllable");
	acceptedDropTypes.add("Container");

	addExistingItems(false);

	commentManagerUI.reset(new CommentManagerViewUI(&manager->commentManager));
	commentManagerUI->canZoom = true;
	commentManagerUI->addExistingItems();
	addAndMakeVisible(commentManagerUI.get(), 0);

	setShowAddButton(false);
}

DashboardItemManagerUI::~DashboardItemManagerUI()
{
	if(!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
	if(DashboardManager::getInstanceWithoutCreating() != nullptr) DashboardManager::getInstance()->snapping->removeAsyncParameterListener(this);
}

void DashboardItemManagerUI::resized()
{
	BaseManagerViewUI::resized();

	addItemBT->setBounds(Rectangle<int>(0, 0, 50, 50));

	if (commentManagerUI != nullptr)
	{
		commentManagerUI->viewOffset = viewOffset;
		commentManagerUI->setBounds(getLocalBounds());
		commentManagerUI->resized();
	}
}

void DashboardItemManagerUI::setViewZoom(float value)
{
	BaseManagerViewUI::setViewZoom(value);
	if (commentManagerUI != nullptr) commentManagerUI->setViewZoom(value);
}

void DashboardItemManagerUI::updateItemsVisibility()
{
	BaseManagerViewUI::updateItemsVisibility();
	if (commentManagerUI != nullptr) commentManagerUI->updateItemsVisibility();
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
	String type = details.description.getProperty("type", "").toString();
	if (type == "Comment")
	{
		commentManagerUI->itemDropped(details);
		return;
	}

	BaseManagerViewUI::itemDropped(details);

	if (details.sourceComponent->getParentComponent() == this) return;

	InspectableContentComponent * icc = dynamic_cast<InspectableContentComponent *>(details.sourceComponent.get());
	if (icc != nullptr && icc->inspectable != nullptr)
	{
		BaseItem* bi = dynamic_cast<BaseItem*>(icc->inspectable.get());
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
	//BaseManagerViewUI::showMenuAndAddItem(fromAddButton, mousePos);

	PopupMenu menu;
	menu.addItem(-1, "Add Group");
	menu.addItem(-2, "Add Comment");
	menu.addSeparator();

	manager->managerFactory->buildPopupMenu();
	menu.addSubMenu("Add Item", manager->managerFactory->getMenu());

	if (int result = menu.show())
	{
		Point<float> p = getViewPos(mousePos);
		if (result == -1) manager->addItem(new DashboardGroupItem(), p);
		else if (result == -2) manager->commentManager.addItem(p);
		else manager->addItem(manager->managerFactory->createFromMenuResult(result), p);
	}
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
