/*
  ==============================================================================

    ScriptTarget.h
    Created: 20 Feb 2017 7:49:11pm
    Author:  Ben

  ==============================================================================
*/

#ifndef SCRIPTTARGET_H_INCLUDED
#define SCRIPTTARGET_H_INCLUDED


const Identifier scriptPtrIdentifier = "_ptr";

class ScriptTarget
{
public:
	ScriptTarget(const String &name, void * ptr) :
		thisPtr((int64)ptr),
		scriptTargetName(name)
	{
		scriptObject.setProperty(scriptPtrIdentifier, thisPtr);
		liveScriptObjectIsDirty = true;
	}

	virtual ~ScriptTarget() {}

	int64 thisPtr;
	String scriptTargetName;
	DynamicObject scriptObject;
	ScopedPointer<DynamicObject> liveScriptObject;
	bool liveScriptObjectIsDirty;

	DynamicObject * getScriptObject()
	{
		if (liveScriptObjectIsDirty)
		{
			updateLiveScriptObject();
		}

		return new DynamicObject(*liveScriptObject);
	}

	void updateLiveScriptObject(DynamicObject * parent = nullptr)
	{
		liveScriptObject = new DynamicObject(scriptObject);

		updateLiveScriptObjectInternal(parent);

		liveScriptObjectIsDirty = false;
		scriptTargetListeners.call(&ScriptTargetListener::scriptObjectUpdated, this);
	}

	virtual void updateLiveScriptObjectInternal(DynamicObject * /*parent*/ = nullptr) {}


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

#endif  // SCRIPTTARGET_H_INCLUDED
