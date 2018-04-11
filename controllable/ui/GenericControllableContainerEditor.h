/*
  ==============================================================================

    GenericControllableContainerEditor.h
    Created: 9 May 2016 6:41:59pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef GENERICCONTROLLABLECONTAINEREDITOR_H_INCLUDED
#define GENERICCONTROLLABLECONTAINEREDITOR_H_INCLUDED

#include "../../inspectable/ui/InspectableEditor.h"
#include "../ControllableContainer.h"
#include "ControllableUI.h"
//#include "../preset/PresetChooser.h"
#include "../ControllableHelpers.h"

class GenericControllableContainerEditor :
	public InspectableEditor,
	public ContainerAsyncListener,
	public Button::Listener,
	public ChangeListener,
	public Label::Listener
{
public:
	GenericControllableContainerEditor(WeakReference<Inspectable> _inspectable, bool isRoot);
	virtual ~GenericControllableContainerEditor();

	
	int headerHeight;
	const int headerGap = 4;
	
	bool prepareToAnimate;
	ComponentAnimator collapseAnimator;

	Colour contourColor;
	Label containerLabel;

	WeakReference<ControllableContainer> container;
	OwnedArray<InspectableEditor> childEditors;

	ScopedPointer<ImageButton> expandBT;
	ScopedPointer<ImageButton> collapseBT;
	Component headerSpacer;

	virtual void setCollapsed(bool value, bool force = false, bool animate = true);
	virtual void resetAndBuild();

	void paint(Graphics &g) override;
	void resized() override;
	virtual void resizedInternal(juce::Rectangle<int> &r);
	virtual void resizedInternalHeader(juce::Rectangle<int> &r);
	virtual void resizedInternalContent(juce::Rectangle<int> &r);
	void clear();

	void mouseDown(const MouseEvent &e) override;

 juce::Rectangle<int> getHeaderBounds(); 
 juce::Rectangle<int> getContentBounds();
	
	bool canBeCollapsed();

	void addControllableUI(Controllable * c, bool resize = false);
	void removeControllableUI(Controllable *c, bool resize = false);

	void addEditorUI(ControllableContainer * cc, bool resize = false);
	void removeEditorUI(ControllableContainer * cc, bool resize = false);
	
	InspectableEditor * getEditorForInspectable(Inspectable * i);
	
	virtual void buttonClicked(Button * b) override;
	virtual void labelTextChanged(Label * l) override;

	void newMessage(const ContainerAsyncEvent & p) override;
	virtual void controllableFeedbackUpdate(Controllable *) {};
	void childBoundsChanged(Component *) override;



	// Inherited via ChangeListener
	virtual void changeListenerCallback(ChangeBroadcaster * source) override;
};


class EnablingControllableContainerEditor :
	public GenericControllableContainerEditor
{
public:
	EnablingControllableContainerEditor(EnablingControllableContainer * cc, bool isRoot);
	~EnablingControllableContainerEditor() {}

	EnablingControllableContainer * ioContainer;
	ScopedPointer<BoolImageToggleUI> enabledUI;

	virtual void resizedInternalHeader(juce::Rectangle<int> &r) override;
	virtual void controllableFeedbackUpdate(Controllable *) override;
};

#endif  // GENERICCONTROLLABLECONTAINEREDITOR_H_INCLUDED
