/*
  ==============================================================================

    GenericControllableItem.h
    Created: 13 May 2017 2:43:46pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class GenericControllableItem :
	public BaseItem
{
public:
	GenericControllableItem(juce::var params = juce::var());
	GenericControllableItem(Controllable* c, juce::var params = juce::var());
	~GenericControllableItem();

	Controllable * controllable = nullptr;

	void onContainerNiceNameChanged() override;
	void controllableNameChanged(Controllable* cn, const juce::String&) override;

	juce::String typeAtCreation;
	virtual juce::String getTypeString() const override { return typeAtCreation; }

	static GenericControllableItem * create(juce::var params) { return new GenericControllableItem(params); }

	InspectableEditor * getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
};