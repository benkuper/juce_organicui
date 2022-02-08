DashboardCCItem::DashboardCCItem(ControllableContainer* container) :
	DashboardInspectableItem(container),
	container(container)
{
	target->targetType = TargetParameter::CONTAINER;
	setInspectable(container);
	ghostInspectable();
}

DashboardCCItem::~DashboardCCItem()
{
	if (container != nullptr) container->removeControllableContainerListener(this);

}

DashboardItemUI* DashboardCCItem::createUI()
{
	return new DashboardCCItemUI(this);
}
void DashboardCCItem::ghostInspectable()
{
	if (!inspectable.wasObjectDeleted() && inspectable != nullptr) inspectableGhostAddress = dynamic_cast<ControllableContainer *>(inspectable.get())->getControlAddress();
}

void DashboardCCItem::checkGhost()
{
	setInspectable(Engine::mainEngine->getControllableContainerForAddress(inspectableGhostAddress));
}

var DashboardCCItem::getServerData()
{
	var data = DashboardInspectableItem::getServerData();
	if (container != nullptr) data.getDynamicObject()->setProperty("container", container->getJSONData());
	return data;
}

var DashboardCCItem::getJSONData()
{
	var data = DashboardInspectableItem::getJSONData();
	if(container != nullptr) data.getDynamicObject()->setProperty("container", container->getControlAddress());
	return data;
}

void DashboardCCItem::loadJSONDataItemInternal(var data)
{

	String address = data.getProperty("container", inspectableGhostAddress);
	if(address.isNotEmpty()) setInspectable(Engine::mainEngine->getControllableContainerForAddress(address));

	DashboardInspectableItem::loadJSONDataItemInternal(data);
}

void DashboardCCItem::setInspectableInternal(Inspectable* i)
{
	if (!inspectable.wasObjectDeleted() && container != nullptr)
	{
		container->removeControllableContainerListener(this);
	}

	container = dynamic_cast<ControllableContainer*>(i);

	if (container != nullptr)
	{
		container->addControllableContainerListener(this);
	}

	target->setValueFromTarget(container);
}

void DashboardCCItem::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (cc == container.get())
	{
		var data(new DynamicObject());
		data.getDynamicObject()->setProperty("controlAddress", c->getControlAddress());
		if (Parameter* p = dynamic_cast<Parameter*>(c)) data.getDynamicObject()->setProperty("value", p->value);
		notifyDataFeedback(data);
	}
	else
	{
		DashboardInspectableItem::onControllableFeedbackUpdateInternal(cc, c);
	}
}
