/*
  ==============================================================================

    ParameterEditor.h
    Created: 7 Oct 2016 2:04:37pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class PlayableParameterAutomationEditor;

class TargetParameterUI;

class ParameterEditor : 
	public ControllableEditor,
	public ParameterListener,
	public Label::Listener,
	public Parameter::AsyncListener
{
public:
	ParameterEditor(Parameter * p, bool isRoot, int initHeight = 16);  //Todo : handle full feedback if is root
	~ParameterEditor();

	WeakReference<Parameter> parameter;

	//Expression
	ScopedPointer<Label> expressionLabel;
	ScopedPointer<Label> expressionText;


	//Target
	ScopedPointer<TargetParameterUI> referenceUI;

	//Automation
	ScopedPointer<PlayableParameterAutomationEditor> automationUI;

	virtual void resized() override;
	virtual void updateUI();

	void newMessage(const Parameter::ParameterEvent &e) override;

	void parameterControlModeChanged(Parameter *) override;


	// Inherited via Listener
	virtual void labelTextChanged(Label * labelThatHasChanged) override;
};