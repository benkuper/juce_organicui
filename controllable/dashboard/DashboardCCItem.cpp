#include "JuceHeader.h"

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
	if (!inspectable.wasObjectDeleted() && inspectable != nullptr) inspectableGhostAddress = dynamic_cast<ControllableContainer*>(inspectable.get())->getControlAddress();
}

void DashboardCCItem::checkGhost()
{
	setInspectable(Engine::mainEngine->getControllableContainerForAddress(inspectableGhostAddress));
}

var DashboardCCItem::getServerData()
{
	var data = DashboardInspectableItem::getServerData();
	if (container != nullptr) data.getDynamicObject()->setProperty("container", getServerDataForContainer(container));
	return data;
}

var DashboardCCItem::getServerDataForContainer(ControllableContainer* cc)
{
	var data(new DynamicObject());

	data.getDynamicObject()->setProperty("name", cc->niceName);
	data.getDynamicObject()->setProperty("id", cc->shortName);
	data.getDynamicObject()->setProperty("type", "container");
	data.getDynamicObject()->setProperty("controlAddress", cc->getControlAddress());

	var contentData(new DynamicObject());

	for (auto& childC : cc->controllables)
	{
		if (childC->hideInRemoteControl) continue;
		contentData.getDynamicObject()->setProperty(childC->shortName, getServerDataForControllable(childC));
	}

	for (auto& childCC : cc->controllableContainers)
	{
		if (childCC->hideInRemoteControl) continue;
		contentData.getDynamicObject()->setProperty(childCC->shortName, getServerDataForContainer(childCC));
	}

	data.getDynamicObject()->setProperty("children", contentData);

	return data;
}

var DashboardCCItem::getServerDataForControllable(Controllable* c)
{
	var data(new DynamicObject());

	data.getDynamicObject()->setProperty("name", c->niceName);
	data.getDynamicObject()->setProperty("id", c->shortName);
	data.getDynamicObject()->setProperty("controlAddress", c->getControlAddress());
	data.getDynamicObject()->setProperty("type", c->getTypeString());
	data.getDynamicObject()->setProperty("readOnly", c->isControllableFeedbackOnly);

	if (Parameter* p = dynamic_cast<Parameter*>(c))
	{
		data.getDynamicObject()->setProperty("value", p->value);

		if (p->hasRange())
		{
			data.getDynamicObject()->setProperty("minVal", p->minimumValue);
			data.getDynamicObject()->setProperty("maxVal", p->maximumValue);
		}

		switch (p->type)
		{
		case Parameter::ENUM:
		{
			var epOptions;
			EnumParameter* ep = (EnumParameter*)p;
			for (auto& ev : ep->enumValues)
			{
				var epData(new DynamicObject());
				epData.getDynamicObject()->setProperty("key", ev->key);
				epData.getDynamicObject()->setProperty("id", ev->value);
				epOptions.append(epData);
			}

			data.getDynamicObject()->setProperty("options", epOptions);
		}
		break;
		case Parameter::POINT2D:
		{
			Point2DParameter* p2d = (Point2DParameter*)p;
			data.getDynamicObject()->setProperty("stretchMode", p2d->extendedEditorStretchMode);
			data.getDynamicObject()->setProperty("invertX", p2d->extendedEditorInvertX);
			data.getDynamicObject()->setProperty("invertY", p2d->extendedEditorInvertY);
		}
		break;


		default:
			break;
		}
	}

	return data;
}

var DashboardCCItem::getJSONData(bool includeNonOverriden)
{
	var data = DashboardInspectableItem::getJSONData(includeNonOverriden);
	if (container != nullptr) data.getDynamicObject()->setProperty("container", container->getControlAddress());
	return data;
}

void DashboardCCItem::loadJSONDataItemInternal(var data)
{

	String address = data.getProperty("container", inspectableGhostAddress);
	if (address.isNotEmpty()) setInspectable(Engine::mainEngine->getControllableContainerForAddress(address));

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
