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
	juce::String controllableType;
	std::function<Controllable*()> createFunc;
	bool isSpecial;

	ControllableDefinition(const juce::String &type, std::function<Controllable*()> createFunc, bool isSpecial = false) :
		controllableType(type),
		createFunc(createFunc),
		isSpecial(isSpecial)
	{}
};

class ControllableFactory
{
public:
	juce_DeclareSingleton(ControllableFactory, true);

	juce::OwnedArray<ControllableDefinition> controllableDefs;
	juce::PopupMenu menu;

	ControllableFactory();
	~ControllableFactory() {}

	void buildPopupMenu(bool excludeSpecials = false);
	juce::PopupMenu getFilteredPopupMenu(juce::StringArray typeFilters, bool excludeSpecials = false);

	static juce::StringArray getTypesWithout(juce::StringArray typesToExclude, bool excludeSpecials = false);

	static void showCreateMenu(std::function<void(Controllable*)> returnFunc, bool excludeSpecials = false);
	static void showFilteredCreateMenu(juce::StringArray typeFilters, std::function<void(Controllable*)> returnFunc, bool excludeSpecials = false);

	static Controllable * createControllable(const juce::String &controllableType);
	static Controllable* createControllableFromJSON(const juce::String &name, juce::var data);

	static Parameter * createParameterFrom(Controllable * source, bool copyName = false, bool copyValue = false);




private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableFactory)
};
