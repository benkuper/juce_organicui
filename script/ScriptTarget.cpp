ScriptTarget::ScriptTarget(const String & name, void * ptr) :
	thisPtr((int64)ptr),
	scriptTargetName(name)
{
	scriptObject.setProperty(scriptPtrIdentifier, thisPtr);
	scriptObject.setMethod(ptrCompareIdentifier, ScriptTarget::checkTargetsAreTheSameFromScript);

	liveScriptObjectIsDirty = true;
}

ScriptTarget::~ScriptTarget() 
{
}

DynamicObject * ScriptTarget::getScriptObject()
{
	if (liveScriptObjectIsDirty)
	{
		updateLiveScriptObject();
	}

	return new DynamicObject(*liveScriptObject);
}

void ScriptTarget::updateLiveScriptObject(DynamicObject * parent)
{
	scriptObjectLock.enter();
	liveScriptObject.reset(new DynamicObject(scriptObject)); //is there a bettery way to deal with updating without recreating an object each time ?
	updateLiveScriptObjectInternal(parent);
	scriptObjectLock.exit();

	liveScriptObjectIsDirty = false;
	scriptTargetListeners.call(&ScriptTargetListener::scriptObjectUpdated, this);
}

var ScriptTarget::checkTargetsAreTheSameFromScript(const var::NativeFunctionArgs & args)
{
	if (args.numArguments == 0) return false;
	if (!args.arguments[0].isObject()) return false;
	DynamicObject * o = args.arguments[0].getDynamicObject();
	if (o == nullptr) return false;
	if (!o->hasProperty(scriptPtrIdentifier)) return false;

	DynamicObject * d = args.thisObject.getDynamicObject();
	if (d == nullptr) return false;
	int64 oId = (int64)o->getProperty(scriptPtrIdentifier);
	int64 thisId = (int64)d->getProperty(scriptPtrIdentifier);

	return oId == thisId;
}
