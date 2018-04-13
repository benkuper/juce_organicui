/*
==============================================================================

Script.h
Created: 20 Feb 2017 5:01:11pm
Author:  Ben

==============================================================================
*/

#ifndef SCRIPT_H_INCLUDED
#define SCRIPT_H_INCLUDED

class Script :
	public BaseItem,
	public MultiTimer,
	public EngineListener
{
public:
	Script(ScriptTarget * parentTarget = nullptr, bool canBeDisabled = true);
	~Script();

	enum ScriptState { SCRIPT_LOADED, SCRIPT_ERROR, SCRIPT_EMPTY, SCRIPT_CLEAR };

	FileParameter * filePath;
	BoolParameter * logParam;
	Trigger * reload;

	ScriptState state;
	String fileName;
	Time fileLastModTime;
	var paramsContainerData; //for keeping overriden values

	bool updateEnabled; //When loading the script, checks if the update function is present
	float lastUpdateTime;
	const Identifier updateIdentifier = "update";

	ControllableContainer scriptParamsContainer;

	ScriptTarget * parentTarget;

	ScopedPointer<JavascriptEngine> scriptEngine;

	void loadScript();
	void buildEnvironment();

	void setState(ScriptState newState);

	bool callFunction(const Identifier &function, const Array<var> args);

	void onContainerParameterChangedInternal(Parameter *) override;
	void onContainerTriggerTriggered(Trigger *) override;

	void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	void endLoadFile() override;
	
	InspectableEditor * getEditor(bool isRoot) override;



	// Inherited via Timer
	virtual void timerCallback(int timerID) override;

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
	static var logFromScript(const var::NativeFunctionArgs &args);

	static var addBoolParameterFromScript(const var::NativeFunctionArgs &args);
	static var addIntParameterFromScript(const var::NativeFunctionArgs &args);
	static var addFloatParameterFromScript(const var::NativeFunctionArgs &args);
	static var addStringParameterFromScript(const var::NativeFunctionArgs &args);
	static var addEnumParameterFromScript(const var::NativeFunctionArgs &args);
	static var addTargetParameterFromScript(const var::NativeFunctionArgs &args);
	static var addColorParameterFromScript(const var::NativeFunctionArgs &args);
	static var addTriggerFromScript(const var::NativeFunctionArgs &args);

	static bool checkNumArgs(const String &logName, const var::NativeFunctionArgs &args, int expectedArgs);


};

#endif  // SCRIPT_H_INCLUDED
