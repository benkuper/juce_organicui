/*
  ==============================================================================

    ParameterEditor.h
    Created: 7 Oct 2016 2:04:37pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ParameterAutomationEditor;
class TargetParameterUI;

class ParameterEditor : 
	public ControllableEditor,
	public ParameterListener,
	public Parameter::AsyncListener
{
public:
	ParameterEditor(juce::Array<Parameter *> parameters, bool isRoot);  //Todo : handle full feedback if is root
	~ParameterEditor();

	juce::Array<juce::WeakReference<Parameter>> parameters;
	juce::WeakReference<Parameter> parameter;
	bool currentUIHasRange;

	//Expression
	std::unique_ptr<juce::Label> expressionLabel;
	std::unique_ptr<juce::Label> expressionText;


	//Target
	std::unique_ptr<TargetParameterUI> referenceUI;

	//Automation
	std::unique_ptr<ParameterAutomationEditor> automationUI;

	virtual void resized() override;
	virtual void updateUI();

	void newMessage(const Parameter::ParameterEvent &e) override;

	virtual void childBoundsChanged(juce::Component* c) override;

	virtual void parameterControlModeChanged(Parameter *) override;

	// Inherited via Listener
	virtual void labelTextChanged(juce::Label * labelThatHasChanged) override;
};