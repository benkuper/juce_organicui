/*
  ==============================================================================

    ControllableEditor.h
    Created: 7 Oct 2016 2:04:37pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLABLEEDITOR_H_INCLUDED
#define CONTROLLABLEEDITOR_H_INCLUDED


class ControllableEditor : 
	public InspectableEditor,
	public ButtonListener,
	public Controllable::Listener
{
public:
	ControllableEditor(Controllable * controllable, bool isRoot, int initHeight = 16);  //Todo : handle full feedback if is root
	virtual ~ControllableEditor();

	Controllable * controllable;
	
	Label label;
	ScopedPointer<ControllableUI> ui;
	ScopedPointer<ImageButton> editBT;
	ScopedPointer<ImageButton> removeBT;
	
	int baseHeight; //height at init
	int subContentHeight; //for additional content

	bool showLabel;
	void setShowLabel(bool value);

	void resized() override;

	void controllableNameChanged(Controllable * override);

    void buttonClicked(Button * b) override;
};



#endif  // CONTROLLABLEEDITOR_H_INCLUDED
