/*
  ==============================================================================

    GenericControllableManager.h
    Created: 13 May 2017 2:31:43pm
    Author:  Ben

  ==============================================================================
*/

#ifndef GENERICCONTROLLABLEMANAGER_H_INCLUDED
#define GENERICCONTROLLABLEMANAGER_H_INCLUDED


class GenericControllableManagerFactory :
	public Factory<GenericControllableItem>
{
public:
	GenericControllableManagerFactory();
};

class GenericControllableManager :
	public BaseManager<GenericControllableItem>
{
public:
	GenericControllableManager(const String &name);
	~GenericControllableManager();

	static GenericControllableManagerFactory factory;

	void addItemFromData(var data, bool fromUndoableAction = false) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericControllableManager)
};



#endif  // GENERICCONTROLLABLEMANAGER_H_INCLUDED
