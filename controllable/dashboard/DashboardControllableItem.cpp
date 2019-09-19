DashboardControllableItem::DashboardControllableItem(Controllable* item) :
	DashboardInspectableItem(item),
	controllable(item)
{
}

DashboardControllableItem::~DashboardControllableItem()
{

}

var DashboardControllableItem::getJSONData()
{
	if (controllable == nullptr) return var(); //not saving for the moment

	var data = DashboardItem::getJSONData();
	data.getDynamicObject()->setProperty("controllable", controllable->getControlAddress());
	return data;
}

void DashboardControllableItem::loadJSONDataItemInternal(var data)
{
	DashboardItem::loadJSONDataItemInternal(data);

	String address = data.getProperty("controllable", "");

	setInspectable(Engine::mainEngine->getControllableForAddress(address));
	if (controllable == nullptr)
	{
		inspectableGhostAddress = address;
		NLOGWARNING("Dashboard", "Item not found when loading : " << address << ", will not be saved");
	}

}


void DashboardControllableItem::setInspectableInternal(Inspectable * i)
{
	controllable = dynamic_cast<Controllable*>(i);
}
