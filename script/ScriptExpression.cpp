/*
==============================================================================

Expression.cpp
Created: 20 Feb 2017 5:01:11pm
Author:  Ben

==============================================================================
*/


ScriptExpression::ScriptExpression() :
	state(EXPRESSION_EMPTY),
	Thread("ScriptExpression")
{
	if(Engine::mainEngine != nullptr) Engine::mainEngine->addScriptTargetListener(this);
}

ScriptExpression::~ScriptExpression()
{
	stopThread(1000);

	if (Engine::mainEngine != nullptr)
	{
		Engine::mainEngine->removeEngineListener(this);
		Engine::mainEngine->removeScriptTargetListener(this);
	}

	for (auto &p : linkedParameters)
	{
		if (p.wasObjectDeleted()) continue;
		p->removeInspectableListener(this);
		p->removeParameterListener(this);
	}
	linkedParameters.clear();
}

void ScriptExpression::setExpression(const String & newExpression)
{
	if (Engine::mainEngine == nullptr)
	{
		stopThread(1000);
		return;
	}

	if (newExpression.isEmpty())
	{
		stopThread(1000);
		state = EXPRESSION_EMPTY;
		return;
	}
	
	expression = newExpression;

	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		Engine::mainEngine->addEngineListener(this);
		return;
	}

	buildEnvironment();
	evaluate(true);

	if (state != EXPRESSION_ERROR)
	{
		setState(EXPRESSION_LOADED);

		if (expression.contains("getTime()"))
		{
			DBG("Start expression timer because expression uses getTime()");
			startThread(); 
		}

	}
}

void ScriptExpression::evaluate(bool resetListeners)
{
	Result result = Result::ok();

	if (resetListeners)
	{
		for (auto &p : linkedParameters)
		{
			if (p.wasObjectDeleted()) continue;
			p->removeInspectableListener(this);
			p->removeParameterListener(this);
		}
		linkedParameters.clear();
	}

	var resultValue = scriptEngine->evaluate(expression,&result);

	if (result.getErrorMessage().isEmpty())
	{
		if (currentValue != resultValue)
		{
			currentValue = resultValue;

			if (resetListeners)
			{
				Array<Parameter *> pList = getParameterReferencesInExpression();
				for (auto &p : pList)
				{
					p->addParameterListener(this);
					p->addInspectableListener(this);
					linkedParameters.add(p);
					LOG("Detected reference in expression to parameter : " << p->niceName);
				}
			}
			
			expressionListeners.call(&ExpressionListener::expressionValueChanged, this);
 		}
	} else
	{
		LOG("Expression error :\n" + result.getErrorMessage());
		setState(EXPRESSION_ERROR);
		stopThread(1000);
		return;
	}


}

void ScriptExpression::buildEnvironment()
{
	//clear phase
	setState(EXPRESSION_CLEAR);
	scriptEngine.reset(new JavascriptEngine());
	//while (scriptParamsContainer.controllables.size() > 0) scriptParamsContainer.removeControllable(scriptParamsContainer.controllables[0]);
	//scriptParamsContainer.clear();

	//scriptEngine->registerNativeObject("script", getScriptObject()); //force "script" for this objet
	//if (parentTarget != nullptr) scriptEngine->registerNativeObject("local", parentTarget->getScriptObject()); //force "local" for the related object
	if (Engine::mainEngine != nullptr) scriptEngine->registerNativeObject(Engine::mainEngine->scriptTargetName, Engine::mainEngine->getScriptObject());
	if (ScriptUtil::getInstanceWithoutCreating() != nullptr) scriptEngine->registerNativeObject(ScriptUtil::getInstance()->scriptTargetName, ScriptUtil::getInstance()->getScriptObject());
}

void ScriptExpression::setState(ExpressionState newState)
{
	state = newState;
	expressionListeners.call(&ExpressionListener::expressionStateChanged, this);
	//scriptAsyncNotifier.addMessage(new ScriptEvent(ScriptEvent::STATE_CHANGE));
}

void ScriptExpression::scriptObjectUpdated(ScriptTarget *)
{
	//if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile) return;

	buildEnvironment();

	//rebuild and reevaluate
	//if (state != EXPRESSION_LOADED) return; //should be WAY more optimized than that !
	//setExpression(expression);
}

void ScriptExpression::endLoadFile()
{
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
	setExpression(expression);
}

Array<Parameter*> ScriptExpression::getParameterReferencesInExpression()
{
	Array<Parameter*> result;
	
	String remainingText = expression;
	Array<StringArray> matches = RegexFunctions::findSubstringsThatMatchWildcard("(?:root|local)\\.([0-9a-zA-Z\\.]+)\\.get\\(\\)", expression); 
	
	for (int i = 0; i < matches.size(); ++i)
	{
		
		String scriptToAddress = "/"+matches[i][1].replaceCharacter('.', '/');
		Controllable * c = Engine::mainEngine != nullptr ? Engine::mainEngine->getControllableForAddress(scriptToAddress) : nullptr;
		Parameter * p = dynamic_cast<Parameter *>(c);
		if (p != nullptr) result.add(p);
	}

	return result;
}

void ScriptExpression::inspectableDestroyed(Inspectable * i)
{
	buildEnvironment();
	//evaluate(true);
}

void ScriptExpression::parameterValueChanged(Parameter * p)
{
	evaluate();
}

void ScriptExpression::run()
{
	while (!threadShouldExit())
	{
		evaluate();
		wait(20); //loop at 50fps only if expression is using time, otherwise relies on parameter value changed
	}
}
