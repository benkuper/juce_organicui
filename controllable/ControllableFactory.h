/*
  ==============================================================================

    ControllableFactory.h
    Created: 2 Nov 2016 1:44:15pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ControllableDefinition
{
public:
	String controllableType;
	std::function<Controllable*()> createFunc;
	bool isSpecial;

	ControllableDefinition(const String &type, std::function<Controllable*()> createFunc, bool isSpecial = false) :
		controllableType(type),
		createFunc(createFunc),
		isSpecial(isSpecial)
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

	void buildPopupMenu(bool excludeSpecials = false);
	PopupMenu getFilteredPopupMenu(StringArray typeFilters, bool excludeSpecials = false);

	static StringArray getTypesWithout(StringArray typesToExclude, bool excludeSpecials = false);

	static Controllable * showCreateMenu(bool excludeSpecials = false);
	static Controllable * showFilteredCreateMenu(StringArray typeFilters, bool excludeSpecials = false);

	static Controllable * createControllable(const String &controllableType);
	static Controllable* createControllableFromJSON(const String &name, var data);

	static Parameter * createParameterFrom(Controllable * source, bool copyName = false, bool copyValue = false);




private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableFactory)
};
