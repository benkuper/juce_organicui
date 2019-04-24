/*
  ==============================================================================

    ControllableEditor.h
    Created: 7 Oct 2016 2:04:37pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ControllableEditor : 
	public InspectableEditor,
	public Button::Listener,
	public Controllable::AsyncListener
{
public:
	ControllableEditor(Controllable * controllable, bool isRoot);  //Todo : handle full feedback if is root
	virtual ~ControllableEditor();

	WeakReference<Controllable> controllable;
	
	Label label;
	ScopedPointer<ControllableUI> ui;
	ScopedPointer<ImageButton> editBT;
	ScopedPointer<ImageButton> removeBT;
	ScopedPointer<ImageButton> enableBT;
	
	int baseHeight; //height at init
	int subContentHeight; //for additional content

	bool showLabel;
	void setShowLabel(bool value);

	void buildControllableUI(bool resizeAfter = false);

	void resized() override;

	void mouseDown(const MouseEvent &e) override;

	void newMessage(const Controllable::ControllableEvent &e) override;

    void buttonClicked(Button * b) override;
};


