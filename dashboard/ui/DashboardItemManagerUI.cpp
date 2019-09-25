#include "DashboardItemManagerUI.h"
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
	//bgColor = Colours::purple;
	//setWantsKeyboardFocus(true);

	setShowAddButton(false);

	manager->addAsyncCoalescedContainerListener(this);

	File f = manager->bgImage->getFile();
	if (f.existsAsFile()) bgImage = ImageCache::getFromFile(f);

	addExistingItems(false);

	acceptedDropTypes.add("Controllable");
	acceptedDropTypes.add("Container");
}

DashboardItemManagerUI::~DashboardItemManagerUI()
{
	if(!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
}

void DashboardItemManagerUI::paint(Graphics& g)
{
	BaseManagerViewUI::paint(g);

	if (manager == nullptr || inspectable.wasObjectDeleted()) return;
	float alpha = manager->bgImageAlpha->floatValue();

	if (bgImage.isNull() || alpha == 0) return;

	g.setColour(Colours::white.withAlpha(alpha));
	float scale = manager->bgImageScale->floatValue();
	Rectangle<float> r = getBoundsInView(Rectangle<float>().withSizeKeepingCentre(bgImage.getWidth()*scale, bgImage.getHeight()*scale));
	g.drawImage(bgImage, r, RectanglePlacement::stretchToFit, false);
	
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

BaseItemMinimalUI<DashboardItem> * DashboardItemManagerUI::createUIForItem(DashboardItem * item)
{
	return item->createUI();
}

void DashboardItemManagerUI::newMessage(const ContainerAsyncEvent& e)
{
	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
		if (e.targetControllable == manager->bgImage)
		{
			File f = manager->bgImage->getFile();
			if (f.existsAsFile()) bgImage = ImageCache::getFromFile(f);
			else bgImage = Image();
			repaint();
		}
		else if (e.targetControllable == manager->bgImageAlpha || manager->bgImageScale)
		{
			repaint();
		}
		break;
	}
}
