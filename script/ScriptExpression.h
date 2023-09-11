/*
==============================================================================

Expression.h
Created: 20 Feb 2017 5:01:11pm
Author:  Ben

==============================================================================
*/

#pragma once

class ScriptExpression :
	public juce::Thread,
	public EngineListener,
	public ScriptTarget::ScriptTargetListener,
	public Inspectable::InspectableListener,
	public ParameterListener
{
public:
	ScriptExpression(Parameter* attachedParam = nullptr);
	~ScriptExpression();

	enum ExpressionState { EXPRESSION_LOADED, EXPRESSION_ERROR, EXPRESSION_EMPTY, EXPRESSION_CLEAR };

	ExpressionState state;
	std::unique_ptr<juce::JavascriptEngine> scriptEngine;

	Parameter* attachedParam;

	juce::String expression;
	juce::var currentValue;

	juce::Array<juce::WeakReference<Parameter>> linkedParameters;

	void setExpression(const juce::String &expression);
	void evaluate(bool resetListeners = false);
	void buildEnvironment();
	void setState(ExpressionState newState);

	void scriptObjectUpdated(ScriptTarget *) override;

	void endLoadFile() override;

	juce::Array<Parameter *> getParameterReferencesInExpression();

	void inspectableDestroyed(Inspectable * i) override;
	void parameterValueChanged(Parameter * p) override;

	juce::ListenerList<ExpressionListener> expressionListeners;
	void addExpressionListener(ExpressionListener* newListener) { expressionListeners.add(newListener); }
	void removeExpressionListener(ExpressionListener* listener) { expressionListeners.remove(listener); }


	// Inherited via Timer
	virtual void run() override;
};
