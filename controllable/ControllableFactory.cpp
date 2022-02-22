/*
  ==============================================================================

	ControllableFactory.cpp
	Created: 2 Nov 2016 1:44:15pm
	Author:  bkupe

  ==============================================================================
*/


juce_ImplementSingleton(ControllableFactory)

ControllableFactory::ControllableFactory() {
	controllableDefs.add(new ControllableDefinition(Trigger::getTypeStringStatic(), &Trigger::create));
	controllableDefs.add(new ControllableDefinition(BoolParameter::getTypeStringStatic(), &BoolParameter::create));
	controllableDefs.add(new ControllableDefinition(FloatParameter::getTypeStringStatic(), &FloatParameter::create));
	controllableDefs.add(new ControllableDefinition(IntParameter::getTypeStringStatic(), &IntParameter::create));
	controllableDefs.add(new ControllableDefinition(EnumParameter::getTypeStringStatic(), &EnumParameter::create));
	controllableDefs.add(new ControllableDefinition(StringParameter::getTypeStringStatic(), &StringParameter::create));
	controllableDefs.add(new ControllableDefinition(FileParameter::getTypeStringStatic(), &FileParameter::create));
	controllableDefs.add(new ControllableDefinition(Point2DParameter::getTypeStringStatic(), &Point2DParameter::create));
	controllableDefs.add(new ControllableDefinition(Point3DParameter::getTypeStringStatic(), &Point3DParameter::create));
	controllableDefs.add(new ControllableDefinition(TargetParameter::getTypeStringStatic(), &TargetParameter::create));
	controllableDefs.add(new ControllableDefinition(ColorParameter::getTypeStringStatic(), &ColorParameter::create));
	buildPopupMenu();
}

void ControllableFactory::buildPopupMenu(bool excludeSpecials)
{
	menu.clear();
	for (int i = 0; i < controllableDefs.size(); ++i)
	{
		if (controllableDefs[i]->isSpecial && excludeSpecials) continue;
		menu.addItem(i + 1, controllableDefs[i]->controllableType);
	}
}

PopupMenu ControllableFactory::getFilteredPopupMenu(StringArray typeFilters, bool excludeSpecials)
{
	PopupMenu result;
	for (int i = 0; i < controllableDefs.size(); ++i)
	{
		if (typeFilters.contains(controllableDefs[i]->controllableType) && (!controllableDefs[i]->isSpecial || !excludeSpecials)) result.addItem(i + 1, controllableDefs[i]->controllableType);
	}

	return result;
}

StringArray ControllableFactory::getTypesWithout(StringArray typesToExclude, bool excludeSpecials)
{
	StringArray result;
	for (auto& d : getInstance()->controllableDefs)
	{
		if (!typesToExclude.contains(d->controllableType) && (!d->isSpecial || !excludeSpecials)) result.add(d->controllableType);
	}
	return result;
}

void ControllableFactory::showCreateMenu(std::function<void(Controllable*)> returnFunc, bool excludeSpecials)
{
	getInstance()->buildPopupMenu(excludeSpecials);

	getInstance()->menu.showMenuAsync(PopupMenu::Options(), [returnFunc](int result)
		{
			if (result == 0) return;

			ControllableDefinition* d = ControllableFactory::getInstance()->controllableDefs[result - 1];//result 0 is no result
			Controllable* c = d->createFunc();
			returnFunc(c);
		}
	);
}

void ControllableFactory::showFilteredCreateMenu(StringArray typeFilters, std::function<void(Controllable*)> returnFunc, bool excludeSpecials)
{
	if (typeFilters.isEmpty()) showCreateMenu(returnFunc, excludeSpecials);

	if (typeFilters.size() == 1)
	{
		for (auto& d : getInstance()->controllableDefs)
		{
			if (d->controllableType == typeFilters[0])
			{
				if (Controllable* c = d->createFunc()) returnFunc(c);
			}
		}
		return;
	}

	PopupMenu filteredMenu = getInstance()->getFilteredPopupMenu(typeFilters, excludeSpecials);

	if (filteredMenu.getNumItems() == 0) return;

	filteredMenu.showMenuAsync(PopupMenu::Options(), [returnFunc](int result)
		{
			if (result == 0) return;

			ControllableDefinition* d = ControllableFactory::getInstance()->controllableDefs[result - 1];//result 0 is no result
			if (Controllable* c = d->createFunc()) returnFunc(c);
		}
	);
}

Controllable* ControllableFactory::createControllable(const String& controllableType)
{
	for (auto& d : getInstance()->controllableDefs)
	{
		if (d->controllableType == controllableType) return d->createFunc();
	}
	return nullptr;
}

Controllable* ControllableFactory::createControllableFromJSON(const String& name, var data)
{
	String valueType = data.getProperty("type", "").toString();
	Controllable* c = ControllableFactory::createControllable(valueType);
	if (c == nullptr) return nullptr;

	c->setNiceName(name);
	c->setupFromJSONData(data);

	return c;
}

Parameter* ControllableFactory::createParameterFrom(Controllable* source, bool copyName, bool copyValue)
{
	Parameter* sourceP = dynamic_cast<Parameter*>(source);
	if (sourceP == nullptr) return nullptr;

	for (auto& d : getInstance()->controllableDefs)
	{
		if (d->controllableType == sourceP->getTypeString())
		{
			Parameter* p = dynamic_cast<Parameter*>(d->createFunc());
			if (copyName) p->setNiceName(source->niceName);
			if (sourceP->hasRange()) p->setRange(sourceP->minimumValue, sourceP->maximumValue);

			if (source->type == Controllable::ENUM)
			{
				EnumParameter* sourceEP = (EnumParameter*)source;
				EnumParameter* ep = (EnumParameter*)p;
				ep->clearOptions();
				for (auto& ev : sourceEP->enumValues) ep->addOption(ev->key, ev->value);
			}
			else if (source->type == Controllable::TARGET)
			{
				TargetParameter* sourceEP = (TargetParameter*)source;
				TargetParameter* tp = (TargetParameter*)p;
				tp->targetType = sourceEP->targetType;
			}

			if (copyValue)
			{
				if (p->type == Controllable::ENUM) p->defaultValue = ((EnumParameter*)sourceP)->getValueKey();
				else p->defaultValue = sourceP->getValue();

				p->resetValue();
			}

			return p;
		}
	}
	return nullptr;
}
