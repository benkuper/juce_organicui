DashboardTriggerItem::DashboardTriggerItem(Trigger * item) :
	DashboardControllableItem(item),
	trigger(item)
{

}

DashboardTriggerItem::~DashboardTriggerItem()
{

}

DashboardItemUI* DashboardTriggerItem::createUI()
{
	return new DashboardTriggerItemUI(this);
}

void DashboardTriggerItem::setInspectableInternal(Inspectable* i)
{
	DashboardControllableItem::setInspectableInternal(i);
	trigger = dynamic_cast<Trigger*>(i);
}
