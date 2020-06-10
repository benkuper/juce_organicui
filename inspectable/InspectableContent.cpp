/*
  ==============================================================================

    InspectableContent.cpp
    Created: 30 Oct 2016 9:08:27am
    Author:  bkupe

  ==============================================================================
*/


InspectableContent::InspectableContent(Inspectable * _inspectable) :
	inspectable(nullptr)
{
	setInspectable(_inspectable);
}

InspectableContent::~InspectableContent()
{
	if(!inspectable.wasObjectDeleted()) inspectable->removeInspectableListener(this);
}

void InspectableContent::setInspectable(Inspectable* i)
{
	if (inspectable == i) return;

	if (inspectable != nullptr && !inspectable.wasObjectDeleted())
	{
		inspectable->removeInspectableListener(this); 
	}

	inspectable = i;

	if (inspectable != nullptr && !inspectable.wasObjectDeleted())
	{
		inspectable->addInspectableListener(this);
	}
}

void InspectableContent::inspectableDestroyed(Inspectable *)
{
	inspectable->removeInspectableListener(this);
}
