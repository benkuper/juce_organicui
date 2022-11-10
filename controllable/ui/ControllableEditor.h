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
	public Controllable::AsyncListener,
	public DragAndDropContainer,
	public Label::Listener
{
public:
	ControllableEditor(Array<Controllable*> controllables, bool isRoot);  //Todo : handle full feedback if is root
	virtual ~ControllableEditor();

	Array<WeakReference<Controllable>> controllables;
	WeakReference<Controllable> controllable;
	
	Label label;
	std::unique_ptr<ControllableUI> ui;
	std::unique_ptr<ImageButton> removeBT;
	std::unique_ptr<ImageButton> enableBT;
	std::unique_ptr<WarningTargetUI> warningUI;
	
	int baseHeight; //height at init
	int subContentHeight; //for additional content
	int minLabelWidth;
	int minControlUIWidth;

	bool dragAndDropEnabled;

	bool showLabel;
	void setShowLabel(bool value);

	virtual void buildControllableUI(bool resizeAfter = false);

	virtual void resized() override;
	virtual void resizedInternal(Rectangle<int>&) {}

	void mouseDown(const MouseEvent &e) override;
	void mouseDrag(const MouseEvent& e) override;


	void newMessage(const Controllable::ControllableEvent &e) override;

	void componentVisibilityChanged(Component& c) override;

    virtual void buttonClicked(Button * b) override;

	// Inherited via Listener
	virtual void labelTextChanged(Label* labelThatHasChanged) override;
};


