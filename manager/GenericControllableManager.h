/*
  ==============================================================================

    GenericControllableManager.h
    Created: 13 May 2017 2:31:43pm
    Author:  Ben

  ==============================================================================
*/
#pragma once

class GenericControllableManager :
	public BaseManager<GenericControllableItem>
{
public:
	GenericControllableManager(const String &name, bool itemsCanBeDisabled = true, bool canAddTriggers = true, bool canAddTargets = true);
	~GenericControllableManager();

	bool itemsCanBeDisabled;
	bool forceItemsFeedbackOnly;

	void setForceItemsFeedbackOnly(bool value);

	Factory<GenericControllableItem> factory;
	void addItemInternal(GenericControllableItem * item, var data) override;
	virtual InspectableEditor * getEditor(bool isRoot) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericControllableManager)
};