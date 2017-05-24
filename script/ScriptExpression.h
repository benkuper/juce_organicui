/*
==============================================================================

Expression.h
Created: 20 Feb 2017 5:01:11pm
Author:  Ben

==============================================================================
*/

#ifndef EXPRESSION_H_INCLUDED
#define EXPRESSION_H_INCLUDED

class ScriptExpression :
	public Timer,
	public EngineListener,
	public ScriptTarget::ScriptTargetListener
{
public:
	ScriptExpression();
	~ScriptExpression();

	enum ExpressionState { EXPRESSION_LOADED, EXPRESSION_ERROR, EXPRESSION_EMPTY, EXPRESSION_CLEAR };

	ExpressionState state;
	ScopedPointer<JavascriptEngine> scriptEngine;

	String expression;
	var currentValue;

	void setExpression(const String &expression);
	void evaluate();
	void buildEnvironment();
	void setState(ExpressionState newState);

	void scriptObjectUpdated(ScriptTarget *) override;

	void endLoadFile() override;

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void expressionValueChanged(ScriptExpression *) {}
		virtual void expressionStateChanged(ScriptExpression *) {}
	};

	ListenerList<Listener> expressionListeners;
	void addExpressionListener(Listener* newListener) { expressionListeners.add(newListener); }
	void removeExpressionListener(Listener* listener) { expressionListeners.remove(listener); }


	// Inherited via Timer
	virtual void timerCallback() override;
};

#endif  // EXPRESSION_H_INCLUDED
