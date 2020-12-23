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
	public Label::Listener,
	public Parameter::AsyncListener
{
public:
	ParameterEditor(Parameter * p, bool isRoot);  //Todo : handle full feedback if is root
	~ParameterEditor();

	WeakReference<Parameter> parameter;
	bool currentUIHasRange;

	//Expression
	std::unique_ptr<Label> expressionLabel;
	std::unique_ptr<Label> expressionText;


	//Target
	std::unique_ptr<TargetParameterUI> referenceUI;

	//Automation
	std::unique_ptr<ParameterAutomationEditor> automationUI;

	virtual void resized() override;
	virtual void updateUI();

	void newMessage(const Parameter::ParameterEvent &e) override;

	virtual void parameterControlModeChanged(Parameter *) override;

	// Inherited via Listener
	virtual void labelTextChanged(Label * labelThatHasChanged) override;
};