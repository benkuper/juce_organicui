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
	GenericControllableManager(const juce::String& name, bool itemsCanBeDisabled = true, bool canAddTriggers = true, bool canAddTargets = true, bool canAddEnums = true);
	~GenericControllableManager();

	bool itemsCanBeDisabled;
	bool forceItemsFeedbackOnly;

	void setForceItemsFeedbackOnly(bool value);

	juce::String getTypeForControllableType(const juce::String& type);

	Factory<GenericControllableItem> factory;

	GenericControllableItem* addItemFrom(Controllable* c, bool copyValue = true);

	void addItemInternal(GenericControllableItem* item, juce::var data) override;
	virtual InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericControllableManager)
};