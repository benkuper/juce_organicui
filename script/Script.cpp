/*
==============================================================================

Script.cpp
Created: 20 Feb 2017 5:01:11pm
Author:  Ben

==============================================================================
*/

#include "JuceHeader.h"
#include "../engine/Engine.h"

Script::Script(ScriptTarget* _parentTarget, bool canBeDisabled, bool canBeRemoved) :
	BaseItem("Script", canBeDisabled, false),
	Thread("Script"),
	forceDisabled(false),
	scriptTemplate(nullptr),
	updateEnabled(false),
	autoRefreshEnvironment(false),
	parentTarget(_parentTarget),
	executionTimeout(5),
	scriptAsyncNotifier(10)
{
	isSelectable = false;

	userCanRemove = canBeRemoved;
	userCanDuplicate = canBeRemoved; //if not removable, then not duplicable
	canBeCopiedAndPasted = canBeRemoved;
	canBeReorderedInEditor = canBeRemoved;

	filePath = new FileParameter("File Path", "Path to the script file", "");
	filePath->setAutoReload(true);
	addParameter(filePath);

	updateRate = addIntParameter("Update Rate", "The Rate at which the \"update()\" function is called", 50, 1, 1000);
	updateRate->hideInEditor = true;

	logParam = addBoolParameter("Log", "Utility parameter to easily activate/deactivate logging from the script", false);
	logParam->setCustomShortName("enableLog");
	logParam->hideInEditor = true;

	reload = addTrigger("Reload", "Reload the script");
	reload->hideInEditor = true;


	scriptObject.getDynamicObject()->setMethod("log", Script::logFromScript);
	scriptObject.getDynamicObject()->setMethod("logWarning", Script::logWarningFromScript);
	scriptObject.getDynamicObject()->setMethod("logError", Script::logErrorFromScript);
	scriptObject.getDynamicObject()->setMethod("setUpdateRate", Script::setUpdateRateFromScript);
	scriptObject.getDynamicObject()->setMethod("setExecutionTimeout", Script::setExecutionTimeoutFromScript);
	scriptObject.getDynamicObject()->setMethod("refreshEnvironment", Script::refreshVariablesFromScript);

	scriptObject.getDynamicObject()->setMethod("addTrigger", Script::addTriggerFromScript);
	scriptObject.getDynamicObject()->setMethod("addBoolParameter", Script::addBoolParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addIntParameter", Script::addIntParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addFloatParameter", Script::addFloatParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addStringParameter", Script::addStringParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addEnumParameter", Script::addEnumParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addTargetParameter", Script::addTargetParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addPoint2DParameter", Script::addPoint2DParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addPoint3DParameter", Script::addPoint3DParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addColorParameter", Script::addColorParameterFromScript);
	scriptObject.getDynamicObject()->setMethod("addFileParameter", Script::addFileParameterFromScript);

	scriptObject.getDynamicObject()->setMethod("setAutoRefreshEnvironment", [](const var::NativeFunctionArgs& args)
		{
			Script* s = getObjectFromJS<Script>(args);
			if (!checkNumArgs(s->niceName, args, 1)) return var();
			s->autoRefreshEnvironment = (bool)args.arguments[0];
			return var();
		});

	setParamsContainer(nullptr); //default

	Engine::mainEngine->addControllableContainerListener(this);

}

Script::~Script()
{
	if (Engine::mainEngine != nullptr)
	{
		Engine::mainEngine->removeControllableContainerListener(this);
	}

	if (scriptEngine != nullptr) scriptEngine->stop();

	stopThread(5000);

	scriptParamsContainer->clear();
}

void Script::setParamsContainer(ControllableContainer* cc)
{
	if (scriptParamsContainer != nullptr)
	{
		removeChildControllableContainer(scriptParamsContainer.get());
		scriptParamsContainer.reset();
	}

	if (cc == nullptr) cc = new ControllableContainer("Params");

	cc->setNiceName("Params");
	scriptParamsContainer.reset(cc);
	scriptParamsContainer->hideEditorHeader = true;
	scriptParamsContainer->saveAndLoadRecursiveData = true;
	addChildControllableContainer(scriptParamsContainer.get());
}

void Script::chooseFileScript(bool openAfter)
{
	FileChooser* chooser(new FileChooser("Create or load a cacahuete", File::getCurrentWorkingDirectory(), "*.js"));

	chooser->launchAsync(FileBrowserComponent::FileChooserFlags::saveMode | FileBrowserComponent::FileChooserFlags::canSelectFiles, [&](const FileChooser& fc)
		{
			File f = fc.getResult();
			delete& fc;

			if (!f.exists())
			{
				f.create();

				if (scriptTemplate != nullptr && scriptTemplate->isNotEmpty())
				{
					FileOutputStream fos(f);
					if (fos.openedOk())
					{
						fos.writeText(*scriptTemplate, false, false, "\n");
						fos.flush();
					}
				}
			}

			filePath->setValue(f.getFullPathName());

			if (f.existsAsFile() && openAfter)
			{
				f.startAsProcess();
			}
		}
	);
}

void Script::loadScript()
{
	if (Engine::mainEngine != nullptr && Engine::mainEngine->isLoadingFile)
	{
		Engine::mainEngine->addEngineListener(this);
		//return;
	}

	if (isThreadRunning())
	{
		stopThread(1000);
	}

	setState(SCRIPT_LOADING);

	availableFunctions.clear();

	String path = filePath->getAbsolutePath();

	if (path.isEmpty())
	{
		return;
	}

#if JUCE_WINDOWS
	if (path.startsWithChar('/')) //avoid jassertfalse
	{
		NLOGWARNING(niceName, "Path " + path + " is not a valid Windows-style path.");
		setState(SCRIPT_ERROR);
		setWarningMessage("Path " + path + " is not a valid Windows-style path.");
		return; //OSX / Linux file
	}
#endif

	File f = filePath->getFile();

	if (!f.exists())
	{
		//check local director
		NLOGWARNING(niceName, "File not found : " + f.getFileName());
		setState(SCRIPT_EMPTY);
		setWarningMessage("File not found : " + f.getFileName());
		return;
	}

	fileName = f.getFileName();
	setNiceName(f.getFileNameWithoutExtension());
	fileLastModTime = f.getLastModificationTime();
	String s = f.loadFileAsString();


	if (paramsContainerData.isVoid()) paramsContainerData = scriptParamsContainer->getJSONData();

	buildEnvironment();

	//	engineLock.enter();
	Result result = scriptEngine->execute(s);
	//	engineLock.exit();

	if (result.getErrorMessage().isEmpty())
	{
		NLOG("Script : " + niceName, "Script loaded succesfully");
		scriptParamsContainer->loadJSONData(paramsContainerData); //keep overriden values
		paramsContainerData = var();

		setState(SCRIPT_LOADED);
		clearWarning();
	}
	else
	{
		NLOGERROR("Script : " + niceName, "Script error : " + result.getErrorMessage());
		setState(SCRIPT_ERROR);
		setWarningMessage("Script error : " + result.getErrorMessage());
		return;
	}

	NamedValueSet nvSet = scriptEngine->getRootObjectProperties();
	for (auto& nv : nvSet)
	{
		String n = nv.name.toString();
		if (n == "script" || n == "local" || n == "util" || n == "root") continue; //reserved names

		if (nv.value.isMethod() || nv.value.isObject())
			availableFunctions.add(n);
	}

	//LOG("available functions : " << availableFunctions.joinIntoString(","));

	updateEnabled = availableFunctions.contains(updateIdentifier);
	updateRate->hideInEditor = !updateEnabled;

	callFunction("init", Array<var>());

	if (updateEnabled)
	{
		if (!Engine::mainEngine->isLoadingFile) startThread();
	}


	scriptParamsContainer->hideInEditor = scriptParamsContainer->controllables.size() == 0;

}

void Script::buildEnvironment()
{
	//clear phase
	setState(SCRIPT_CLEAR);

	//if (scriptEngine == nullptr)
	//{
	scriptEngine.reset(new JavascriptEngine());
	scriptEngine->maximumExecutionTime = RelativeTime::seconds(executionTimeout);
	//}
	//else
	//{
		//scriptEngine->stop();
	//}

	while (scriptParamsContainer->controllables.size() > 0) scriptParamsContainer->removeControllable(scriptParamsContainer->controllables[0]);
	scriptParamsContainer->clear();

	refreshVariables();
}

void Script::refreshVariables()
{
	scriptEngine->registerNativeObject("script", getScriptObject().getDynamicObject()); //force "script" for this object
	if (parentTarget != nullptr) scriptEngine->registerNativeObject("local", parentTarget->getScriptObject().getDynamicObject()); //force "local" for the related object
	if (Engine::mainEngine != nullptr) scriptEngine->registerNativeObject(Engine::mainEngine->scriptTargetName, Engine::mainEngine->getScriptObject().getDynamicObject());
	if (ScriptUtil::getInstanceWithoutCreating() != nullptr)
	{
		DynamicObject* utilObject = ScriptUtil::getInstance()->getScriptObject().getDynamicObject();
		utilObject->setProperty("_script", (int64)this);
		utilObject->setProperty("scriptPath", filePath->getAbsolutePath());
		scriptEngine->registerNativeObject(ScriptUtil::getInstance()->scriptTargetName, utilObject);
	}
}

void Script::setState(ScriptState newState)
{
	state = newState;
	scriptAsyncNotifier.addMessage(new ScriptEvent(ScriptEvent::STATE_CHANGE));
}

var Script::callFunction(const Identifier& function, const Array<var> args, Result* result, bool skipIfAlreadyCalling)
{
	if (canBeDisabled && (!enabled->boolValue() || forceDisabled)) return false;
	if (scriptEngine == nullptr) return false;

	if (!availableFunctions.contains(function.toString()))
	{
		//NLOGERROR(niceName, "Function " + function.toString() + " not found in script");
		return var();
	}

	Result tmpResult = Result::ok();
	if (result == nullptr) result = &tmpResult;

	//const ScopedLock sl(engineLock); //TODO : This is causing deadlock when multiple scripts are triggering controllableFeedbackUpdate. Need to find a better way to handle feedback update, maybe with the new ThreadSafe and Lightweight listeners

	var returnData;
	if (skipIfAlreadyCalling)
	{
		ScopedTryLock lock(scriptCallLock);
		if (!lock.isLocked()) return var();
		returnData = scriptEngine->callFunction(function, var::NativeFunctionArgs(getScriptObject(), (const var*)args.begin(), args.size()), result);
	}
	else
	{
		GenericScopedLock lock(scriptCallLock);
		returnData = scriptEngine->callFunction(function, var::NativeFunctionArgs(getScriptObject(), (const var*)args.begin(), args.size()), result);
	}

	if (result->getErrorMessage().isNotEmpty())
	{
		NLOGERROR(niceName, "Script Error :\n" + result->getErrorMessage());
		return var();
	}

	return returnData;
}


void Script::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == filePath)
	{
		if (!isCurrentlyLoadingData) loadScript();
	}
}

void Script::onContainerTriggerTriggered(Trigger* t)
{
	if (t == reload)
	{
		loadScript();
	}
}

void Script::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (Engine::mainEngine->isClearing) return;

	if (cc == scriptParamsContainer.get())
	{
		Array<var> args;
		args.add(c->getScriptObject());

		callFunction("scriptParameterChanged", args);
	}
}

void Script::childStructureChanged(ControllableContainer* cc)
{
	if (cc != Engine::mainEngine) BaseItem::childStructureChanged(cc);

	if (state == ScriptState::SCRIPT_LOADED && Engine::mainEngine != nullptr && cc == Engine::mainEngine)
	{
		if (!Engine::mainEngine->isLoadingFile && !Engine::mainEngine->isClearing)
		{
			//const ScopedLock sl(engineLock); //need to check null MainEngine again after lock
			//if (Engine::mainEngine != nullptr && scriptEngine != nullptr)
			//	Engine::mainEngine->getScriptObject(); //force update if needed

			if (autoRefreshEnvironment) refreshVariables();
		}
	}
}

var Script::getJSONData(bool includeNonOverriden)
{
	var data = BaseItem::getJSONData(includeNonOverriden);
	var pData = scriptParamsContainer->getJSONData();
	if (!pData.isVoid()) data.getDynamicObject()->setProperty("scriptParams", pData);
	return data;
}

void Script::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	paramsContainerData = data.getProperty("scriptParams", var());
	loadScript();
}

void Script::endLoadFile()
{
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
	if (state != SCRIPT_LOADED) loadScript();
	else if (updateEnabled)
	{
		startThread();
	}
}

InspectableEditor* Script::getEditorInternal(bool isRoot, Array<Inspectable*> inspectables)
{
	return new ScriptEditor(this, isRoot);
}

void Script::checkScriptFile()
{
	//check for modifications

	File f = filePath->getFile();
	if (f.getLastModificationTime() != fileLastModTime)
	{
		loadScript();
	}
}

void Script::run()
{
	double millisAtLastUpdate = Time::getMillisecondCounterHiRes();

	while (!Engine::mainEngine->isClearing && !threadShouldExit() && state == ScriptState::SCRIPT_LOADED && updateEnabled)
	{
		double nowMillis = Time::getMillisecondCounterHiRes();
		double deltaMillis = nowMillis - millisAtLastUpdate;
		millisAtLastUpdate = nowMillis;

		Array<var> args;
		args.add(deltaMillis / 1000.0);

		Result r = Result::ok();
		callFunction(updateIdentifier, args, &r, true);
		if (r != Result::ok()) return;

		double afterProcessMillis = Time::getMillisecondCounterHiRes();
		double processTime = afterProcessMillis - nowMillis;

		wait(jmax<int>((1000.0 / updateRate->floatValue()) - processTime, 0));
	}
}

void Script::logFromArgs(const var::NativeFunctionArgs& args, int logLevel)
{
	Script* s = ScriptTarget::getObjectFromJS<Script>(args);
	if (!s->logParam->boolValue()) return;

	String logS = "";
	for (int i = 0; i < args.numArguments; ++i)
	{
		var a = args.arguments[i];
		String as = ScriptUtil::getLogStringForVar(a);
		logS += (i > 0 ? "\n" : "") + as;
	}

	switch (logLevel)
	{
	case 0: NLOG("Script : " + s->niceName, logS); break;
	case 1: NLOGWARNING("Script : " + s->niceName, logS); break;
	case 2: NLOGERROR("Script : " + s->niceName, logS); break;
	}
}

var Script::logFromScript(const var::NativeFunctionArgs& args)
{
	logFromArgs(args, 0);
	return var();
}

var Script::logWarningFromScript(const var::NativeFunctionArgs& args)
{
	logFromArgs(args, 1);
	return var();
}

var Script::logErrorFromScript(const var::NativeFunctionArgs& args)
{
	logFromArgs(args, 2);
	return var();
}

var Script::addTriggerFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 2)) return var();
	return s->scriptParamsContainer->addTrigger(args.arguments[0], args.arguments[1])->getScriptObject();
}

var Script::addBoolParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 3)) return var();
	Parameter* p = s->scriptParamsContainer->addBoolParameter(args.arguments[0], args.arguments[1], (bool)args.arguments[2]);
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addIntParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 3)) return var();
	Parameter* p = s->scriptParamsContainer->addIntParameter(args.arguments[0], args.arguments[1], (int)args.arguments[2], args.numArguments >= 4 ? (int)args.arguments[3] : INT32_MIN, args.numArguments >= 5 ? (int)args.arguments[4] : INT32_MAX);
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addFloatParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 3)) return var();
	Parameter* p = s->scriptParamsContainer->addFloatParameter(args.arguments[0], args.arguments[1], (float)args.arguments[2], args.numArguments >= 4 ? (int)args.arguments[3] : INT32_MIN, args.numArguments >= 5 ? (int)args.arguments[4] : INT32_MAX);
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addStringParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 3)) return var();
	Parameter* p = s->scriptParamsContainer->addStringParameter(args.arguments[0], args.arguments[1], args.arguments[2]);
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addEnumParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 2)) return var();
	EnumParameter* p = s->scriptParamsContainer->addEnumParameter(args.arguments[0], args.arguments[1]);
	int numOptions = (int)floor((args.numArguments - 2) / 2.0f);
	for (int i = 0; i < numOptions; ++i)
	{
		int optionIndex = 2 + i * 2;
		p->addOption(args.arguments[optionIndex].toString(), args.arguments[optionIndex + 1]);
	}
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addTargetParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 2)) return var();

	TargetParameter* tp = s->scriptParamsContainer->addTargetParameter(args.arguments[0], args.arguments[1]);
	if (args.numArguments >= 3)
	{
		bool isContainer = (int)args.arguments[2] > 0;
		if (isContainer) tp->targetType = TargetParameter::CONTAINER;
	}
	tp->isCustomizableByUser = true;
	return tp->getScriptObject();
}

var Script::addColorParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 3)) return var();
	var color;
	if (args.arguments[2].isArray())
	{
		color = args.arguments[2];
		while (color.size() < 4) color.append(color.size() < 3 ? 0 : 1);
	}
	else if (args.numArguments >= 5)
	{
		color.append((float)args.arguments[2]);
		color.append((float)args.arguments[3]);
		color.append((float)args.arguments[4]);
		color.append((float)args.numArguments >= 6 ? (float)args.arguments[5] : 1.f);
	}
	else if (args.arguments[2].isInt() || args.arguments[2].isInt64())
	{
		color.append((float)(((int)args.arguments[2] >> 24) & 0xFF) / 255.f);
		color.append((float)(((int)args.arguments[2] >> 16) & 0xFF) / 255.f);
		color.append((float)(((int)args.arguments[2] >> 8) & 0xFF) / 255.f);
		color.append((float)(((int)args.arguments[2]) & 0xFF) / 255.f);
	}

	Parameter* p = s->scriptParamsContainer->addColorParameter(args.arguments[0], args.arguments[1], Colour((float)color[0], (float)color[1], (float)color[2], (float)color[3]));
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addPoint2DParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 2)) return var();
	Parameter* p = s->scriptParamsContainer->addPoint2DParameter(args.arguments[0], args.arguments[1]);
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addPoint3DParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 2)) return var();
	Parameter* p = s->scriptParamsContainer->addPoint3DParameter(args.arguments[0], args.arguments[1]);
	p->isCustomizableByUser = true;
	return p->getScriptObject();
}

var Script::addFileParameterFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 2)) return var();
	FileParameter* fp = s->scriptParamsContainer->addFileParameter(args.arguments[0], args.arguments[1]);
	fp->directoryMode = args.numArguments > 2 ? ((int)args.arguments[2] > 0) : false;
	fp->isCustomizableByUser = true;
	return fp->getScriptObject();
}


var Script::setUpdateRateFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 1)) return var();
	if (s->updateEnabled) s->updateRate->setValue(args.arguments[0]);

	return var();
}

var Script::setExecutionTimeoutFromScript(const var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	if (!checkNumArgs(s->niceName, args, 1)) return var();
	s->executionTimeout = (float)args.arguments[0];
	if (s->scriptEngine != nullptr) s->scriptEngine->maximumExecutionTime = RelativeTime::seconds(s->executionTimeout);

	return var();
}

var Script::refreshVariablesFromScript(const juce::var::NativeFunctionArgs& args)
{
	Script* s = getObjectFromJS<Script>(args);
	s->refreshVariables();
	return var();
}


bool Script::checkNumArgs(const String& logName, const var::NativeFunctionArgs& args, int expectedArgs)
{
	if (args.numArguments < expectedArgs)
	{
		NLOG(logName, "Error, function takes at least" + String(expectedArgs) + " arguments, got " + String(args.numArguments));
		if (args.numArguments > 0) NLOG("", "When tying to add : " + args.arguments[0].toString());
		return false;
	}

	return true;
}