/*
 ==============================================================================

 Controllable.h
 Created: 8 Mar 2016 1:08:56pm
 Author:  bkupe

 ==============================================================================
 */

#pragma once

class ControllableUI;
class ControllableContainer;
class DashboardItem;
class ControllableDetectiveWatcher;

class Controllable :
	public Inspectable,
	public ScriptTarget,
	public DashboardItemTarget
{
public:
	enum Type { CUSTOM, TRIGGER, FLOAT, INT, BOOL, STRING, ENUM, POINT2D, POINT3D, TARGET, COLOR, TYPE_MAX };
	static const juce::Array<juce::String> typeNames;
	static const juce::Array<juce::String> remoteControlTypeNames;

	Controllable(const Type& type, const juce::String& niceName, const juce::String& description, bool enabled = true);
	virtual ~Controllable();

	Type type;
	juce::String niceName;
	juce::String shortName;
	juce::String description;
	juce::String argumentsDescription;

	bool enabled;
	bool canBeDisabledByUser;
	bool descriptionIsEditable;
	bool hasCustomShortName;
	bool isControllableFeedbackOnly;
	bool includeInScriptObject;

	juce::String controlAddress;

	//save & load
	bool isSavable;
	bool saveValueOnly;
	bool isLoadingData;
	bool isControlledByParrot;

	//user control
	bool isCustomizableByUser;
	bool isRemovableByUser;
	bool userCanChangeName;
	bool userCanSetReadOnly;

	bool replaceSlashesInShortName;

	//dashboard
	int dashboardDefaultLabelParentLevel; //used to show another 
	bool dashboardDefaultAppendLabel;		//if default parent level is not 0 and this this will 
	//decide if this controllable's label will be added at the end


	juce::WeakReference<ControllableContainer> parentContainer;

	juce::UndoableAction* setUndoableNiceName(const juce::String& _niceName, bool onlyReturnAction = false);
	void setNiceName(const juce::String& _niceName);
	void setCustomShortName(const juce::String& _shortName);
	void setAutoShortName();

	virtual void setEnabled(bool value, bool silentSet = false, bool force = false);
	virtual void setControllableFeedbackOnly(bool value);

	void notifyStateChanged();

	void setParentContainer(ControllableContainer* container);

	template<class T>
	T* getParentAs() {
		if (parentContainer == nullptr || parentContainer.wasObjectDeleted()) return nullptr;
		return dynamic_cast<T*>(parentContainer.get());
	}
	void updateControlAddress();

	void remove(bool addToUndo = false); // called from external to make this object ask for remove

	virtual void updateScriptObjectInternal(juce::var parent = juce::var()) override;

	virtual bool shouldBeSaved();

	virtual juce::var getJSONData(ControllableContainer* relativeTo = nullptr);
	virtual juce::var getJSONDataInternal() { return juce::var(new juce::DynamicObject()); } // to be overriden
	virtual void loadJSONData(juce::var data);
	virtual void loadJSONDataInternal(juce::var data) {} //to be overriden

	virtual void setupFromJSONData(juce::var data);

	//Remote control
	virtual juce::var getRemoteControlData();
	virtual void getRemoteControlDataInternal(juce::var&/*data*/) {}

	juce::String getControlAddress(ControllableContainer* relativeTo = nullptr);

	// used for generating editor
	virtual ControllableUI* createDefaultUI(juce::Array<Controllable*> = {}) {
		jassertfalse;
		return nullptr;
	}

	virtual DashboardItem* createDashboardItem() override;
	virtual juce::String getDefaultDashboardLabel() const;

	virtual void setAttribute(juce::String param, juce::var value);
	virtual bool setAttributeInternal(juce::String param, juce::var value);
	virtual juce::var getAttribute(juce::String param) const;
	virtual juce::var getAttributeInternal(juce::String param) const;
	virtual juce::StringArray getValidAttributes() const;

	static juce::var setValueFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var checkIsParameterFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getParentFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var setNameFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var setAttributeFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getAttributeFromScript(const juce::var::NativeFunctionArgs& a);

	static juce::var getControlAddressFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getScriptControlAddressFromScript(const juce::var::NativeFunctionArgs& a);

	static juce::var getJSONDataFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var loadJSONDataFromScript(const juce::var::NativeFunctionArgs& args);


	juce::String getScriptTargetString() override;

	virtual juce::String getWarningTargetName() const override;

	bool isAttachedToRoot();

	virtual InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
	virtual ControllableDetectiveWatcher* getDetectiveWatcher();
	virtual juce::String getTypeString() const { jassert(false); return ""; } //should be overriden


	class ControllableListener
	{
	public:
		/** Destructor. */
		virtual ~ControllableListener() {}
		virtual void controllableStateChanged(Controllable*) {}
		virtual void controllableFeedbackStateChanged(Controllable*) {}
		virtual void controllableControlAddressChanged(Controllable*) {}
		virtual void controllableNameChanged(Controllable*) {}
		virtual void controllableAttributeChanged(Controllable*, const juce::String&) {}
		virtual void askForRemoveControllable(Controllable*, bool /*addToUndo*/ = false) {}
	};

	DECLARE_INSPECTACLE_CRITICAL_LISTENER(Controllable, controllable);
	DECLARE_ASYNC_EVENT(Controllable, Controllable, controllable, ENUM_LIST(STATE_CHANGED, FEEDBACK_STATE_CHANGED, CONTROLADDRESS_CHANGED, NAME_CHANGED, ATTRIBUTE_CHANGED, ASK_FOR_REMOVE, CONTROLLABLE_REMOVED), EVENT_INSPECTABLE_CHECK);

private:
	juce::WeakReference<Controllable>::Master masterReference;
	friend class juce::WeakReference<Controllable>;


public:
	class ControllableAction :
		public juce::UndoableAction
	{
	public:
		ControllableAction(Controllable* c) :
			controllableRef(c)
		{
			controlAddress = c->getControlAddress();
		}

		juce::WeakReference<Controllable> controllableRef;
		juce::String controlAddress;

		Controllable* getControllable();
	};

	class ControllableChangeNameAction :
		public ControllableAction
	{
	public:
		ControllableChangeNameAction(Controllable* c, juce::String oldName, juce::String newName) :
			ControllableAction(c),
			oldName(oldName),
			newName(newName)
		{
		}

		juce::String oldName;
		juce::String newName;

		bool perform() override;
		bool undo() override;
	};

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controllable)
};
