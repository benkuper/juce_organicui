DashboardControllableItem::DashboardControllableItem(Controllable* item) :
	DashboardInspectableItem(item),
	controllable(item)
{
	showLabel = addBoolParameter("Show Label", "If checked, label is shown on controller", true);
	textColor = addColorParameter("Text Color", "Color of the text", TEXT_COLOR, false);
	customLabel = addStringParameter("Custom text", "If not empty, will override the label of this control", "", false);
	customDescription = addStringParameter("Custom description", "If not empty, will override the description of this control","", false);

	textColor->canBeDisabledByUser = true;
	customLabel->canBeDisabledByUser = true;
	customDescription->canBeDisabledByUser = true;
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
