/*
  ==============================================================================

    ParrotUI.cpp
    Created: 4 Oct 2021 1:03:57pm
    Author:  bkupe

  ==============================================================================
*/


ParrotUI::ParrotUI(Parrot* p) :
    ItemUI(p)
{
    updateBGColor();
}

ParrotUI::~ParrotUI()
{
}

void ParrotUI::controllableFeedbackUpdateInternal(Controllable* c)
{
    ItemUI::controllableFeedbackUpdateInternal(c);
    if (c == item->status)
    {
        updateBGColor();
    }
}

void ParrotUI::updateBGColor()
{
    Parrot::Status s = item->status->getValueDataAsEnum<Parrot::Status>();
    switch (s)
    {
    case Parrot::Status::IDLE: bgColor = BG_COLOR.brighter(.1f); break;
    case Parrot::Status::RECORDING: bgColor = RED_COLOR.brighter(.2f); break;
    case Parrot::Status::PLAYING: bgColor = Colours::rebeccapurple.brighter(.2f); break;
    }
    repaint();
}

bool ParrotUI::isInterestedInDragSource(const DragAndDropTarget::SourceDetails& d)
{
    String type = d.description.getProperty("dataType", "");
    return type == "Controllable" || type == "Container";
}

void ParrotUI::itemDropped(const DragAndDropTarget::SourceDetails& d)
{
    String type = d.description.getProperty("dataType", "");
    if (type == "Controllable")
    {
        if (ControllableEditor* ce = dynamic_cast<ControllableEditor*>(d.sourceComponent.get()))
        {
            item->addTarget(ce->controllable.get());
        }
        else if (ControllableUI* cui = dynamic_cast<ControllableUI*>(d.sourceComponent.get()))
        {
            item->addTarget(ce->controllable.get());
        }
    }
    else if (type == "Container")
    {
        if(GenericControllableContainerEditor * gce = dynamic_cast<GenericControllableContainerEditor*>(d.sourceComponent.get()))
        {
            ControllableContainer* cc = gce->container.get();
            for(auto & c : cc->controllables) item->addTarget(c);
        }
    }

    ItemUI::itemDropped(d);
}
