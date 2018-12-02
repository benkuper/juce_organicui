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
	PopupMenu getFilteredPopupMenu(StringArray typeFilters);

	static StringArray getTypesWithout(StringArray typesToExclude);

	static Controllable * showCreateMenu();
	static Controllable * showFilteredCreateMenu(StringArray typeFilters);

	static Controllable * createControllable(const String &controllableType);

	static Parameter * createParameterFrom(Controllable * source, bool copyName = false, bool copyValue = false);




private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableFactory)
};
