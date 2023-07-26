/*
  ==============================================================================

    ScriptTarget.h
    Created: 20 Feb 2017 7:49:11pm
    Author:  Ben

  ==============================================================================
*/


#pragma once

const juce::Identifier scriptPtrIdentifier = "_ptr";
const juce::Identifier scriptTargetTypeIdentifier = "_type";
const juce::Identifier ptrCompareIdentifier = "is";

class ScriptTarget
{
public:
	ScriptTarget(const juce::String &name, void * ptr, const juce::String &targetType = "");
	virtual ~ScriptTarget();

	juce::int64 thisPtr;
	juce::String scriptTargetName;
	juce::var scriptObject;
	//var liveScriptObject;
	bool scriptObjectIsDirty;

	juce::SpinLock scriptObjectLock;

	juce::var getScriptObject();
	void updateScriptObject(juce::var parent = juce::var());

	virtual void updateScriptObjectInternal(juce::var /*parent*/ = juce::var()) {}

	static juce::var checkTargetsAreTheSameFromScript(const juce::var::NativeFunctionArgs &args);

	virtual juce::String getScriptTargetString();

	class ScriptTargetListener
	{
	public:
		virtual ~ScriptTargetListener() {};
		virtual void scriptObjectUpdated(ScriptTarget *) {};
	};

	juce::ListenerList<ScriptTargetListener> scriptTargetListeners;
	void addScriptTargetListener(ScriptTargetListener * l) { scriptTargetListeners.add(l); }
	void removeScriptTargetListener(ScriptTargetListener * l) { scriptTargetListeners.remove(l); }



	template<class T>
	static T* getObjectFromJS(const juce::var::NativeFunctionArgs & a);
};

template<class T>
T * ScriptTarget::getObjectFromJS(const juce::var::NativeFunctionArgs & a) {
	juce::DynamicObject * d = a.thisObject.getDynamicObject();
	if (d == nullptr) return nullptr;
	return dynamic_cast<T*>((T*)(juce::int64)d->getProperty(scriptPtrIdentifier));
}