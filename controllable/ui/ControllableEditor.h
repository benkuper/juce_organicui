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
	public juce::Button::Listener,
	public Controllable::AsyncListener,
	public juce::DragAndDropContainer,
	public juce::Label::Listener
{
public:
	ControllableEditor(juce::Array<Controllable*> controllables, bool isRoot);  //Todo : handle full feedback if is root
	virtual ~ControllableEditor();

	juce::Array<juce::WeakReference<Controllable>> controllables;
	juce::WeakReference<Controllable> controllable;
	
	juce::Label label;
	std::unique_ptr<ControllableUI> ui;
	std::unique_ptr<juce::ImageButton> removeBT;
	std::unique_ptr<juce::ImageButton> enableBT;
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
	virtual void resizedInternal(juce::Rectangle<int>&) {}

	void mouseDown(const juce::MouseEvent &e) override;
	void mouseDrag(const juce::MouseEvent& e) override;


	void newMessage(const Controllable::ControllableEvent &e) override;

	void componentVisibilityChanged(juce::Component& c) override;

    virtual void buttonClicked(juce::Button * b) override;

	// Inherited via Listener
	virtual void labelTextChanged(juce::Label* labelThatHasChanged) override;
};


