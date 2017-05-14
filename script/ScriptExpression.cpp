/*
==============================================================================

Expression.cpp
Created: 20 Feb 2017 5:01:11pm
Author:  Ben

==============================================================================
*/

ScriptExpression::ScriptExpression() :
	state(EXPRESSION_EMPTY)
{
}

ScriptExpression::~ScriptExpression()
{

}

void ScriptExpression::setExpression(const String & newExpression)
{
	if (newExpression.isEmpty())
	{
		stopTimer();
		state = EXPRESSION_EMPTY;
		return;
	}
	
	expression = newExpression;
	buildEnvironment(); 
	
	evaluate();

	if(state != EXPRESSION_ERROR) setState(EXPRESSION_LOADED);

	startTimerHz(30); //30 fps loop time, to do : find a way to loop only when inner value change
}

void ScriptExpression::evaluate()
{
	Result result = Result::ok();
	var resultValue = scriptEngine->evaluate(expression,&result);

	if (result.getErrorMessage().isEmpty())
	{
		if (currentValue != resultValue)
		{
			currentValue = resultValue;
			expressionListeners.call(&Listener::expressionValueChanged, this);
 		}
	} else
	{
		LOG("Expression error :\n" + result.getErrorMessage());
		setState(EXPRESSION_ERROR);
		stopTimer();
		return;
	}
}

void ScriptExpression::buildEnvironment()
{
	//clear phase
	setState(EXPRESSION_CLEAR);
	scriptEngine = new JavascriptEngine();
	//while (scriptParamsContainer.controllables.size() > 0) scriptParamsContainer.removeControllable(scriptParamsContainer.controllables[0]);
	//scriptParamsContainer.clear();

	//scriptEngine->registerNativeObject("script", createScriptObject()); //force "script" for this objet
	//if (parentTarget != nullptr) scriptEngine->registerNativeObject("local", parentTarget->createScriptObject()); //force "local" for the related object
	if (Engine::mainEngine != nullptr) scriptEngine->registerNativeObject(Engine::mainEngine->scriptTargetName, Engine::mainEngine->createScriptObject());
	if (ScriptUtil::getInstanceWithoutCreating() != nullptr) scriptEngine->registerNativeObject(ScriptUtil::getInstance()->scriptTargetName, ScriptUtil::getInstance()->createScriptObject());
}

void ScriptExpression::setState(ExpressionState newState)
{
	state = newState;
	expressionListeners.call(&Listener::expressionStateChanged, this);
	//scriptAsyncNotifier.addMessage(new ScriptEvent(ScriptEvent::STATE_CHANGE));
}

void ScriptExpression::timerCallback()
{
	evaluate();
}
