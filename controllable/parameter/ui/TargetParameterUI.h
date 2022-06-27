/*
  ==============================================================================

	TargetParameterUI.h
	Created: 2 Nov 2016 5:00:10pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class TargetStepButton
{
public:
	TargetStepButton(const String& name, WeakReference<ControllableContainer> reference);
	TextButton bt;
	WeakReference<ControllableContainer> reference;
};

class TargetParameterUI :
	public ParameterUI,
	public Button::Listener,
	public ContainerAsyncListener
{
public:
	TargetParameterUI(Array<TargetParameter*> parameters, const String& noTargetText = "[Click to select an element]");
	virtual ~TargetParameterUI();

	String noTargetText;
	Label label;
	std::unique_ptr<ImageButton> targetBT;

	Array<TargetParameter*> targetParameters;
	TargetParameter* targetParameter;

	std::unique_ptr<BoolParameter> listeningToNextChange;
	std::unique_ptr<BoolToggleUI> listeningToNextChangeBT;

	std::unique_ptr<ControllableChooserPopupMenu> controllableChooser;
	std::unique_ptr<ContainerChooserPopupMenu> containerChooser;

	bool useCustomShowFullAddressInEditor;
	bool customShowFullAddressInEditor;
	bool useCustomShowParentNameInEditor;
	bool customShowParentNameInEditor;
	int customParentLabelSearch;
	bool useCustomShowLearnButton;
	bool customShowLearnButton;

	
	OwnedArray<TargetStepButton> stepsUI;

	void paint(Graphics& g) override;
	void resized() override;

	virtual void updateLabel();
	virtual void updateUIParamsInternal() override;

	virtual void showPopupAndGetTarget(ControllableContainer* startFromCC = nullptr);//can be overriden to get specific PopupMenu
	void mouseDownInternal(const MouseEvent& e) override;

	virtual void buttonClicked(Button* b) override;

protected:
	void valueChanged(const var& v) override;
	void newMessage(const Parameter::ParameterEvent& e) override;
	void newMessage(const ContainerAsyncEvent& e) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TargetParameterUI)
};