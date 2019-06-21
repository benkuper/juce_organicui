#include "ControllableFactory.h"
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

void ControllableFactory::buildPopupMenu()
{
	for (int i = 0; i < controllableDefs.size(); i++)
	{
		menu.addItem(i + 1, controllableDefs[i]->controllableType);
	}
}

PopupMenu ControllableFactory::getFilteredPopupMenu(StringArray typeFilters)
{
	PopupMenu result;
	for (int i = 0; i < controllableDefs.size(); i++)
	{
		if(typeFilters.contains(controllableDefs[i]->controllableType)) result.addItem(i + 1, controllableDefs[i]->controllableType);
	}

	return result;
}

StringArray ControllableFactory::getTypesWithout(StringArray typesToExclude)
{
	StringArray result;
	for (auto &d : getInstance()->controllableDefs)
	{
		if (!typesToExclude.contains(d->controllableType)) result.add(d->controllableType);
	}
	return result;
}

Controllable * ControllableFactory::showCreateMenu()
{
	int result = getInstance()->menu.show();
	if (result == 0) return nullptr;
	else
	{
		ControllableDefinition * d = getInstance()->controllableDefs[result - 1];//result 0 is no result
		return d->createFunc();
	}
}

Controllable * ControllableFactory::showFilteredCreateMenu(StringArray typeFilters)
{
	if (typeFilters.isEmpty()) return showCreateMenu();

	if (typeFilters.size() == 1)
	{
		for (auto &d : getInstance()->controllableDefs)
		{
			if (d->controllableType == typeFilters[0]) return d->createFunc();
		}
		return nullptr;
	}

	PopupMenu filteredMenu = getInstance()->getFilteredPopupMenu(typeFilters);
	
	if (filteredMenu.getNumItems() == 0) return nullptr;
	int result = filteredMenu.show();
	if (result == 0) return nullptr;
	else
	{
		ControllableDefinition * d = getInstance()->controllableDefs[result - 1];//result 0 is no result
		return d->createFunc();
	}
}

Controllable * ControllableFactory::createControllable(const String & controllableType)
{
	for (auto &d : getInstance()->controllableDefs)
	{
		if (d->controllableType == controllableType) return d->createFunc();
	}
	return nullptr;
}

Parameter * ControllableFactory::createParameterFrom(Controllable * source, bool copyName, bool copyValue)
{
	Parameter * sourceP = dynamic_cast<Parameter *>(source);
	if (sourceP == nullptr) return nullptr;

	for (auto &d : getInstance()->controllableDefs)
	{
		if (d->controllableType == sourceP->getTypeString())
		{
			Parameter * p = dynamic_cast<Parameter *>(d->createFunc());
			if (copyName) p->setNiceName(source->niceName);
			if(sourceP->hasRange()) p->setRange(sourceP->minimumValue, sourceP->maximumValue);

			if (source->type == Controllable::ENUM)
			{
				EnumParameter * sourceEP = (EnumParameter *)source;
				EnumParameter * ep = (EnumParameter *)p;
				ep->clearOptions();
				for (auto &ev : sourceEP->enumValues) ep->addOption(ev->key, ev->value);
			}
			else if (source->type == Controllable::TARGET)
			{
				TargetParameter* sourceEP = (TargetParameter*)source;
				TargetParameter* tp = (TargetParameter*)p;
				tp->targetType = sourceEP->targetType;
			}

			if (copyValue)
			{
				p->defaultValue = sourceP->getValue();
				p->resetValue();
			}

			return p;
		}
	}
	return nullptr;
}
