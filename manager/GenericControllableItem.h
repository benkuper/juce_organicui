/*
  ==============================================================================

    GenericControllableItem.h
    Created: 13 May 2017 2:43:46pm
    Author:  Ben

  ==============================================================================
*/

#ifndef GENERICCONTROLLABLEITEM_H_INCLUDED
#define GENERICCONTROLLABLEITEM_H_INCLUDED

class GenericControllableItem :
	public BaseItem
{
public:
	GenericControllableItem(var params = var());
	~GenericControllableItem();

	Controllable * controllable;

	void onContainerNiceNameChanged() override;

	String typeAtCreation;
	virtual String getTypeString() const override { return typeAtCreation; }

	static GenericControllableItem * create(var params) { return new GenericControllableItem(params); }

	InspectableEditor * getEditor(bool isRoot) override;
};



#endif  // GENERICCONTROLLABLEITEM_H_INCLUDED
