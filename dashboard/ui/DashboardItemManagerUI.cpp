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

	addExistingItems(false);
	acceptedDropTypes.add("Controllable");
	acceptedDropTypes.add("Container");
}

DashboardItemManagerUI::~DashboardItemManagerUI()
{
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
