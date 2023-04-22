/*
  ==============================================================================

    ScriptTarget.h
    Created: 20 Feb 2017 7:49:11pm
    Author:  Ben

  ==============================================================================
*/


#pragma once

const Identifier scriptPtrIdentifier = "_ptr";
const Identifier scriptTargetTypeIdentifier = "_type";
const Identifier ptrCompareIdentifier = "is";

class ScriptTarget
{
public:
	ScriptTarget(const String &name, void * ptr, const String &targetType = "");
	virtual ~ScriptTarget();

	int64 thisPtr;
	String scriptTargetName;
	var scriptObject;
	//var liveScriptObject;
	bool scriptObjectIsDirty;

	SpinLock scriptObjectLock;

	var getScriptObject();
	void updateScriptObject(var parent = var());

	virtual void updateScriptObjectInternal(var /*parent*/ = var()) {}

	static var checkTargetsAreTheSameFromScript(const var::NativeFunctionArgs &args);

	virtual String getScriptTargetString();

	class ScriptTargetListener
	{
	public:
		virtual ~ScriptTargetListener() {};
		virtual void scriptObjectUpdated(ScriptTarget *) {};
	};

	ListenerList<ScriptTargetListener> scriptTargetListeners;
	void addScriptTargetListener(ScriptTargetListener * l) { scriptTargetListeners.add(l); }
	void removeScriptTargetListener(ScriptTargetListener * l) { scriptTargetListeners.remove(l); }



	template<class T>
	static T* getObjectFromJS(const var::NativeFunctionArgs & a);
};

template<class T>
T * ScriptTarget::getObjectFromJS(const var::NativeFunctionArgs & a) {
	DynamicObject * d = a.thisObject.getDynamicObject();
	if (d == nullptr) return nullptr;
	return dynamic_cast<T*>((T*)(int64)d->getProperty(scriptPtrIdentifier));
}