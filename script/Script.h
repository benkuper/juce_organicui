/*
==============================================================================

Script.h
Created: 20 Feb 2017 5:01:11pm
Author:  Ben

==============================================================================
*/

#pragma once


class Script :
	public BaseItem,
	public juce::Timer,
	public juce::Thread,
	public EngineListener
{
public:
	Script(ScriptTarget* parentTarget = nullptr, bool canBeDisabled = true, bool canBeRemoved = true);
	~Script();

	enum ScriptState { SCRIPT_LOADING, SCRIPT_LOADED, SCRIPT_ERROR, SCRIPT_EMPTY, SCRIPT_CLEAR };

	bool forceDisabled; //for scripts that can not be disabled by users directly

	juce::String* scriptTemplate;
	FileParameter* filePath;
	BoolParameter* logParam;
	Trigger* reload;
	IntParameter* updateRate;

	ScriptState state;
	juce::String fileName;
	juce::Time fileLastModTime;
	juce::var paramsContainerData; //for keeping overriden values

	bool updateEnabled; //When loading the script, checks if the update function is present
	const juce::Identifier updateIdentifier = "update";

	std::unique_ptr<ControllableContainer> scriptParamsContainer;

	ScriptTarget* parentTarget;

	juce::StringArray availableFunctions;

	std::unique_ptr<juce::JavascriptEngine> scriptEngine;
	juce::CriticalSection engineLock;
	float executionTimeout;


	void setParamsContainer(ControllableContainer* cc);
	void chooseFileScript(bool openAfter = false);

	void loadScript();
	void buildEnvironment();

	void setState(ScriptState newState);


	juce::var callFunction(const juce::Identifier& function, const juce::Array<juce::var> args, juce::Result* result = (juce::Result*)nullptr);

	void onContainerParameterChangedInternal(Parameter*) override;
	void onContainerTriggerTriggered(Trigger*) override;

	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;
	void childStructureChanged(ControllableContainer* cc) override;

	juce::var getJSONData() override;
	void loadJSONDataInternal(juce::var data) override;

	void endLoadFile() override;

	InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;


	// Inherited via Timer
	virtual void timerCallback() override;

	virtual void run() override;

	class ScriptEvent
	{
	public:
		enum Type { STATE_CHANGE };
		ScriptEvent(Type _type) : type(_type) {}
		Type type;
	};

	QueuedNotifier<ScriptEvent> scriptAsyncNotifier;
	typedef QueuedNotifier<ScriptEvent>::Listener AsyncListener;


	void addAsyncScriptListener(AsyncListener* newListener) { scriptAsyncNotifier.addListener(newListener); }
	void addAsyncCoalescedScriptListener(AsyncListener* newListener) { scriptAsyncNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncScriptListener(AsyncListener* listener) { scriptAsyncNotifier.removeListener(listener); }

	//Script functions
	static void logFromArgs(const juce::var::NativeFunctionArgs& args, int logLevel);
	static juce::var logFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var logWarningFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var logErrorFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var addTriggerFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addBoolParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addIntParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addFloatParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addStringParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addEnumParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addTargetParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addColorParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addPoint2DParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addPoint3DParameterFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var addFileParameterFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var setUpdateRateFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var setExecutionTimeoutFromScript(const juce::var::NativeFunctionArgs& args);

	static bool checkNumArgs(const juce::String& logName, const juce::var::NativeFunctionArgs& args, int expectedArgs);


};
