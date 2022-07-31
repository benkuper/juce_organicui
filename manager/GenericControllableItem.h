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
	GenericControllableItem(var params = var());
	GenericControllableItem(Controllable* c, var params = var());
	~GenericControllableItem();

	Controllable * controllable = nullptr;

	void onContainerNiceNameChanged() override;
	void controllableNameChanged(Controllable* c) override;

	String typeAtCreation;
	virtual String getTypeString() const override { return typeAtCreation; }

	static GenericControllableItem * create(var params) { return new GenericControllableItem(params); }

	InspectableEditor * getEditorInternal(bool isRoot, Array<Inspectable*> inspectables = Array<Inspectable*>()) override;
};