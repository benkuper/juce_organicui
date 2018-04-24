/*
  ==============================================================================

    ControllableFactory.h
    Created: 2 Nov 2016 1:44:15pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLABLEFACTORY_H_INCLUDED
#define CONTROLLABLEFACTORY_H_INCLUDED

class ControllableDefinition
{
public:
	String controllableType;
	std::function<Controllable*()> createFunc;

	ControllableDefinition(const String &type, std::function<Controllable*()> createFunc) :
		controllableType(type),
		createFunc(createFunc)
	{}
};

class ControllableFactory
{
public:
	juce_DeclareSingleton(ControllableFactory, true);

	OwnedArray<ControllableDefinition> controllableDefs;
	PopupMenu menu;

	ControllableFactory();
	~ControllableFactory() {}

	void buildPopupMenu();

	static Controllable * showCreateMenu()
	{
		int result = getInstance()->menu.show();
		if (result == 0) return nullptr;
		else
		{
			ControllableDefinition * d = getInstance()->controllableDefs[result - 1];//result 0 is no result
			return d->createFunc();
		}
	}

	static Controllable * createControllable(const String &controllableType)
	{
		for (auto &d : getInstance()->controllableDefs)
		{
			if (d->controllableType == controllableType) return d->createFunc();
		}
		return nullptr;
	}

	static Parameter * createParameterFrom(Controllable * source, bool copyName = false, bool copyValue = false)
	{
		Parameter * sourceP = dynamic_cast<Parameter *>(source);
		if (sourceP == nullptr) return nullptr;

		for (auto &d : getInstance()->controllableDefs)
		{
			if (d->controllableType == sourceP->getTypeString())
			{
				Parameter * p = dynamic_cast<Parameter *>(d->createFunc());
				if (copyName) p->setNiceName(source->niceName);
				p->setRange(sourceP->minimumValue, sourceP->maximumValue);
				if (copyValue)
				{
					p->defaultValue = sourceP->value;
					p->resetValue();
				}

				return p;
			}
		}
		return nullptr;
	}




	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableFactory)
};

#endif  // CONTROLLABLEFACTORY_H_INCLUDED
