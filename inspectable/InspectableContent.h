/*
  ==============================================================================

    InspectableContent.h
    Created: 30 Oct 2016 9:08:27am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "Inspectable.h"

class InspectableContent :
	public Inspectable::InspectableListener
{
public:
	InspectableContent(Inspectable * inspectable);
	virtual ~InspectableContent();

	WeakReference<Inspectable> inspectable;

	virtual void setInspectable(Inspectable* i);

	void inspectableDestroyed(Inspectable *) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectableContent)
};