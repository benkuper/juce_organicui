/*
  ==============================================================================

    ParrotUI.h
    Created: 4 Oct 2021 1:03:57pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParrotUI :
    public BaseItemUI<Parrot>
{
public:
    ParrotUI(Parrot* p);
    ~ParrotUI();

    void controllableFeedbackUpdateInternal(Controllable* c) override;
    
    void updateBGColor();

    bool isInterestedInDragSource(const DragAndDropTarget::SourceDetails& d) override;
    void itemDropped(const DragAndDropTarget::SourceDetails& d) override;

    
};