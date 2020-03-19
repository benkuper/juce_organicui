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
	juce::DynamicObject scriptObject;
	std::unique_ptr<juce::DynamicObject> liveScriptObject;
	bool liveScriptObjectIsDirty;

	SpinLock scriptObjectLock;

	juce::DynamicObject * getScriptObject();
	void updateLiveScriptObject(juce::DynamicObject * parent = nullptr);

	virtual void updateLiveScriptObjectInternal(juce::DynamicObject * /*parent*/ = nullptr) {}

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