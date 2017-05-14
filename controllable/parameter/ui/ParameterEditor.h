/*
  ==============================================================================

    ParameterEditor.h
    Created: 7 Oct 2016 2:04:37pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef PARAMETEREDITOR_H_INCLUDED
#define PARAMETEREDITOR_H_INCLUDED


class ParameterEditor : 
	public ControllableEditor,
	public Parameter::Listener,
	public Label::Listener
{
public:
	ParameterEditor(Parameter * p, bool isRoot, int initHeight = 16);  //Todo : handle full feedback if is root
	~ParameterEditor();

	Parameter * parameter;
	ScopedPointer<Label> expressionLabel;
	ScopedPointer<Label> expressionText;

	virtual void resized() override;
	virtual void updateUI();

	void parameterControlModeChanged(Parameter *) override;

	// Inherited via Listener
	virtual void labelTextChanged(Label * labelThatHasChanged) override;
};



#endif  // PARAMETEREDITOR_H_INCLUDED
