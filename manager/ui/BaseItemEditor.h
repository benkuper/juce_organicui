/*
  ==============================================================================

    BaseItemEditor.h
    Created: 18 Jan 2017 2:23:31pm
    Author:  Ben

  ==============================================================================
*/

#ifndef BASEITEMEDITOR_H_INCLUDED
#define BASEITEMEDITOR_H_INCLUDED


class BaseItemEditor :
	public EnablingControllableContainerEditor
{
public:
	BaseItemEditor(BaseItem * bi, bool isRoot);
	~BaseItemEditor();

	BaseItem * item;

	bool isFirst;
	bool isLast;

	ScopedPointer<ImageButton> removeBT;
	ScopedPointer<ImageButton> upBT;
	ScopedPointer<ImageButton> downBT;

	void setIsFirst(bool value);
	void setIsLast(bool value);

	virtual void resizedInternalHeader(juce::Rectangle<int> & r) override;
	virtual void resizedInternalHeaderItemInternal(juce::Rectangle<int> &) {}
	virtual void buttonClicked(Button *b) override;

};




#endif  // BASEITEMEDITOR_H_INCLUDED
