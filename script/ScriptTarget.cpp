#include "JuceHeader.h"

ScriptTarget::ScriptTarget(const String& name, void* ptr, const String& targetType) :
	thisPtr((int64)ptr),
	scriptTargetName(name)
{
	scriptObject = new DynamicObject();
	scriptObject.getDynamicObject()->setProperty(scriptPtrIdentifier, thisPtr);
	scriptObject.getDynamicObject()->setProperty(scriptTargetTypeIdentifier, targetType);
	scriptObject.getDynamicObject()->setMethod(ptrCompareIdentifier, ScriptTarget::checkTargetsAreTheSameFromScript);
	//scriptObjectIsDirty = true;
}

ScriptTarget::~ScriptTarget()
{
}

var ScriptTarget::getScriptObject()
{
	//if (scriptObjectIsDirty) updateScriptObject();
	return scriptObject;
}

//void ScriptTarget::updateScriptObject()
//{
	//scriptObjectLock.enter();
	//liveScriptObject = scriptObject.clone(); //is there a bettery way to deal with updating without recreating an object each time ?
	//updateScriptObjectInternal();
	//scriptObjectLock.exit();

	//scriptObjectIsDirty = false;
	//scriptTargetListeners.call(&ScriptTargetListener::scriptObjectUpdated, this);
//}

var ScriptTarget::checkTargetsAreTheSameFromScript(const var::NativeFunctionArgs& args)
{
	if (args.numArguments == 0) return false;
	if (!args.arguments[0].isObject()) return false;
	DynamicObject* o = args.arguments[0].getDynamicObject();
	if (o == nullptr) return false;
	if (!o->hasProperty(scriptPtrIdentifier)) return false;

	DynamicObject* d = args.thisObject.getDynamicObject();
	if (d == nullptr) return false;
	int64 oId = (int64)o->getProperty(scriptPtrIdentifier);
	int64 thisId = (int64)d->getProperty(scriptPtrIdentifier);

	return oId == thisId;
}

String ScriptTarget::getScriptTargetString()
{
	return "[" + scriptTargetName + "]";
}
