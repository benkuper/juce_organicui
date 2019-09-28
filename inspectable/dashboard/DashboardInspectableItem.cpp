DashboardInspectableItem::DashboardInspectableItem(Inspectable* item) :
	DashboardItem(item),
	inspectable(nullptr),
	inspectableItemNotifier(20)
{
	if(item != nullptr) setInspectable(item);
}

DashboardInspectableItem::~DashboardInspectableItem()
{
}

void DashboardInspectableItem::clearItem()
{
	setInspectable(nullptr);
	Engine::mainEngine->removeControllableContainerListener(this);
}

void DashboardInspectableItem::setInspectable(Inspectable* i)
{
	if (inspectable == i) return;
	
	if (!inspectable.wasObjectDeleted() && inspectable != nullptr)
	{
		inspectable->removeInspectableListener(this);
	}

	inspectable = i;

	if (inspectable != nullptr)
	{
		ghostInspectable();
		inspectable->addInspectableListener(this);
		Engine::mainEngine->removeControllableContainerListener(this);
	}

	setInspectableInternal(inspectable);
	
	inspectableItemNotifier.addMessage(new InspectableItemEvent(InspectableItemEvent::INSPECTABLE_CHANGED, inspectable));
}

void DashboardInspectableItem::inspectableDestroyed(Inspectable* i)
{
	DashboardItem::inspectableDestroyed(i);

	if (!Engine::mainEngine->isClearing && i == inspectable)
	{
		Engine::mainEngine->addControllableContainerListener(this);
		setInspectable(nullptr);
	}
}

void DashboardInspectableItem::childStructureChanged(ControllableContainer* cc)
{
	if (cc == Engine::mainEngine)
	{
		if (inspectable.wasObjectDeleted() || inspectable == nullptr)
		{
			checkGhost();
		}

		return;
	}
	
	DashboardItem::childStructureChanged(cc);
}

var DashboardInspectableItem::getJSONData()
{
	var data = DashboardItem::getJSONData();
	if (inspectableGhostAddress.isNotEmpty()) data.getDynamicObject()->setProperty("ghostAddress", inspectableGhostAddress);
	return data;
}

void DashboardInspectableItem::loadJSONDataItemInternal(var data)
{
	DashboardItem::loadJSONDataItemInternal(data);

	inspectableGhostAddress = data.getProperty("ghostAddress", inspectableGhostAddress);
	if (inspectable.wasObjectDeleted() || inspectable == nullptr)
	{
		Engine::mainEngine->addControllableContainerListener(this);
	}
}
