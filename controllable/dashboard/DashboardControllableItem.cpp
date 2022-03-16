DashboardControllableItem::DashboardControllableItem(Controllable* item) :
	DashboardInspectableItem(item),
	controllable(item),
	dashboardItemNotifier(5)
{
	if (item != nullptr)
	{
		Point2DParameter* dp = nullptr;
		switch (item->type)
		{
		case Controllable::TRIGGER: dp = ProjectSettings::getInstance()->triggerDefaultSize; break;
		case Controllable::BOOL: dp = ProjectSettings::getInstance()->boolDefaultSize; break;
		case Controllable::FLOAT: dp = ProjectSettings::getInstance()->floatDefautSize; break;
		case Controllable::INT: dp = ProjectSettings::getInstance()->intDefautSize; break;
		case Controllable::STRING: dp = ProjectSettings::getInstance()->stringDefaultSize; break;
		case Controllable::COLOR: dp = ProjectSettings::getInstance()->colorDefaultSize; break;
		case Controllable::ENUM: dp = ProjectSettings::getInstance()->enumDefaultSize; break;
		case Controllable::TARGET: dp = ProjectSettings::getInstance()->targetDefaultSize; break;
		case Controllable::POINT2D: dp = ProjectSettings::getInstance()->p2dDefaultSize; break;
		case Controllable::POINT3D: dp = ProjectSettings::getInstance()->p3dDefaultSize; break;
		default:
			break;
		}

		if (dp != nullptr && dp->enabled) viewUISize->setPoint(dp->getPoint());
		else viewUISize->setPoint(100, 20);
	}
	else
	{
		viewUISize->setPoint(100, 20);
	}


	showLabel = addBoolParameter("Show Label", "If checked, label is shown on controller", true);
	customLabel = addStringParameter("Custom text", "If not empty, will override the label of this control", "", false);

	textColor = addColorParameter("Text Color", "Color of the text", TEXT_COLOR, false);
	textSize = addIntParameter("Text Size", "Size of the text. Auto size if disabled", 12, 2, 100, false);
	contourColor = addColorParameter("Border Color", "Color of the contour", BG_COLOR.brighter(), false);
	contourThickness = addFloatParameter("Border Width", "Thickness of the contour", 2, 1);
	opaqueBackground = addBoolParameter("Opaque Background", "If checked, background is opaque", true);
	customDescription = addStringParameter("Custom description", "If not empty, will override the description of this control", "", false);

	forceReadOnly = addBoolParameter("Read Only", "If not already read-only, this will force not being able to edit it from the dashboard", false);

	textColor->canBeDisabledByUser = true;
	textSize->canBeDisabledByUser = true;
	contourColor->canBeDisabledByUser = true;
	customLabel->canBeDisabledByUser = true;
	customDescription->canBeDisabledByUser = true;

	if (controllable != nullptr && !controllable.wasObjectDeleted())
	{
		if (controllable->dashboardDefaultLabelParentLevel > 0)
		{
			customLabel->setEnabled(true);
			customLabel->setValue(controllable->getDefaultDashboardLabel());
		}

		controllable->addControllableListener(this);
	}
}

DashboardControllableItem::~DashboardControllableItem()
{
	if (inspectable != nullptr && !inspectable.wasObjectDeleted()) controllable->removeControllableListener(this);
	dashboardItemNotifier.cancelPendingUpdate();
}

var DashboardControllableItem::getItemParameterFeedback(Parameter* p)
{
	var data = DashboardItem::getItemParameterFeedback(p);
	data.getDynamicObject()->setProperty("targetType", controllable != nullptr ? controllable->getTypeString() : "");
	return data;
}

var DashboardControllableItem::getJSONData()
{
	var data = DashboardInspectableItem::getJSONData();
	if (controllable != nullptr) data.getDynamicObject()->setProperty("controllable", controllable->getControlAddress());
	return data;
}

void DashboardControllableItem::loadJSONData(var data, bool createIfNotThere)
{
	if (inspectable == nullptr)
	{
		String address = data.getProperty("controllable", inspectableGhostAddress);
		setInspectable(Engine::mainEngine->getControllableForAddress(address));
	}

	DashboardInspectableItem::loadJSONData(data, createIfNotThere);
}


void DashboardControllableItem::setInspectableInternal(Inspectable* i)
{
	if (controllable != nullptr && !controllable.wasObjectDeleted())
	{
		controllable->removeControllableListener(this);
	}

	controllable = dynamic_cast<Controllable*>(i);

	if (controllable != nullptr && !controllable.wasObjectDeleted())
	{
		controllable->addControllableListener(this);
	}

	target->setValueFromTarget(controllable);
}

void DashboardControllableItem::ghostInspectable()
{
	if (controllable != nullptr && !controllable.wasObjectDeleted())
	{
		if (ControllableUtil::findParentAs<Engine>(controllable) != nullptr) //ensure this controllable is attached to the global structure
		{
			inspectableGhostAddress = controllable->getControlAddress();
		}
	}
}

void DashboardControllableItem::checkGhost()
{
	setInspectable(Engine::mainEngine->getControllableForAddress(inspectableGhostAddress));
}


void DashboardControllableItem::controllableStateChanged(Controllable* c)
{
	if (c != controllable) ControllableContainer::controllableStateChanged(c);
}

void DashboardControllableItem::controllableFeedbackStateChanged(Controllable* c)
{
	if (c != controllable) ControllableContainer::controllableFeedbackStateChanged(c);
}

void DashboardControllableItem::controllableControlAddressChanged(Controllable* c)
{
	if (c == controllable)
	{
		ghostInspectable();
		dashboardItemNotifier.addMessage(new DashboardControllableItemEvent(DashboardControllableItemEvent::NEEDS_UI_UPDATE, this));
	}
	else
	{
		ControllableContainer::controllableControlAddressChanged(c);
	}
}

void DashboardControllableItem::controllableNameChanged(Controllable* c)
{
	if (c != controllable) ControllableContainer::controllableNameChanged(c);
}

void DashboardControllableItem::askForRemoveControllable(Controllable* c, bool addToUndo)
{
	if (c != controllable) ControllableContainer::askForRemoveControllable(c, addToUndo);
}

var DashboardControllableItem::getServerData()
{
	var data = DashboardInspectableItem::getServerData();
	DynamicObject* o = data.getDynamicObject();

	o->setProperty("id", shortName);

	if (controllable == nullptr || controllable.wasObjectDeleted())
	{
		o->setProperty("ghostAddress", inspectableGhostAddress);
		return var(o);
	}

	o->setProperty("type", controllable->getTypeString());
	o->setProperty("controlAddress", controllable->getControlAddress());

	o->setProperty("showLabel", showLabel->value);
	if (textColor->enabled) o->setProperty("textColor", textColor->value);
	if (textSize->enabled) o->setProperty("textSize", textSize->value);
#
	o->setProperty("borderColor", contourColor->value);
	o->setProperty("borderWidth", contourThickness->value);
	o->setProperty("borderColorEnabled", contourColor->enabled);

	o->setProperty("opaqueBackground", opaqueBackground->value);
	o->setProperty("label", controllable->getDefaultDashboardLabel());
	o->setProperty("customTextEnabled", customLabel->enabled);
	o->setProperty("customText", customLabel->value.toString());
	o->setProperty("customDescription", customDescription->value);
	o->setProperty("readOnly", forceReadOnly->value || controllable->isControllableFeedbackOnly);

	return data;
}
