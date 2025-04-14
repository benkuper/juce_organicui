/*
  ==============================================================================

	ControllableContainer.h
	Created: 8 Mar 2016 1:15:36pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ControllableContainer :
	public ParameterListener,
	public Controllable::ControllableListener,
	public Parameter::AsyncListener,
	public Trigger::TriggerListener,
	public ControllableContainerListener,
	public Inspectable,
	public ScriptTarget,
	public DashboardItemTarget,
	public WarningTarget::AsyncListener

{
public:
	ControllableContainer(const juce::String& niceName);
	virtual ~ControllableContainer();

	juce::String niceName;
	juce::String shortName;
	bool hasCustomShortName;
	bool allowSameChildrenNiceNames;

	bool nameCanBeChangedByUser;

	bool isClearing;

	//Editor
	bool canInspectChildContainers;
	bool editorIsCollapsed;
	bool editorCanBeCollapsed;
	bool hideEditorHeader;
	bool skipLabelInTarget;
	bool userCanAddControllables;
	bool isRemovableByUser;

	std::function<void(ControllableContainer*)> customUserCreateControllableFunc;
	juce::StringArray userAddControllablesFilters;


	std::function<InspectableEditor* (bool, juce::Array<ControllableContainer*>)> customGetEditorFunc;

	//save / load
	bool saveAndLoadRecursiveData;
	bool saveAndLoadName;
	bool includeInRecursiveSave;
	bool includeTriggersInSaveLoad;
	bool isCurrentlyLoadingData;
	bool notifyStructureChangeWhenLoadingData;
	bool isNotifyingStructureChange;
	bool canBeCopiedAndPasted;

	//Script
	bool includeInScriptObject;

	//Remote control
	bool notifyRemoteControlOnLoad;
	bool notifyRemoteControlOnClear;


	static ControllableComparator comparator;
	ControllableComparator* customControllableComparator;

	juce::OwnedArray<Controllable, juce::CriticalSection> controllables;
	juce::Array<juce::WeakReference<ControllableContainer>, juce::CriticalSection> controllableContainers;
	juce::OwnedArray<ControllableContainer, juce::CriticalSection> ownedContainers;
	juce::WeakReference<ControllableContainer> parentContainer;

	juce::UndoableAction* setUndoableNiceName(const juce::String& _niceName, bool onlyReturnAction = false);
	void setNiceName(const juce::String& _niceName);
	void setCustomShortName(const juce::String& _shortName);
	void setAutoShortName();


	juce::UndoableAction* addUndoableControllable(Controllable* c, bool onlyReturnAction = false);
	Controllable* addControllable(Controllable* c, int index = -1);
	Parameter* addParameter(Parameter* p, int index = -1);
	FloatParameter* addFloatParameter(const juce::String& niceName, const juce::String& description, const double& initialValue, const double& minValue = INT32_MIN, const double& maxValue = INT32_MAX, const bool& enabled = true);
	IntParameter* addIntParameter(const juce::String& niceName, const juce::String& description, const int& initialValue, const int& minValue = INT32_MIN, const int& maxValue = INT32_MAX, const bool& enabled = true);
	BoolParameter* addBoolParameter(const juce::String& niceName, const juce::String& description, const bool& value, const bool& enabled = true);
	StringParameter* addStringParameter(const juce::String& niceName, const juce::String& description, const juce::String& value, const bool& enabled = true);
	EnumParameter* addEnumParameter(const juce::String& niceName, const juce::String& description, const bool& enabled = true);
	Point2DParameter* addPoint2DParameter(const juce::String& niceName, const juce::String& description, const bool& enabled = true);
	Point3DParameter* addPoint3DParameter(const juce::String& niceName, const juce::String& description, const bool& enabled = true);
	ColorParameter* addColorParameter(const juce::String& niceName, const juce::String& description, const juce::Colour& initialColor, const bool& enabled = true);
	TargetParameter* addTargetParameter(const juce::String& niceName, const juce::String& description, juce::WeakReference<ControllableContainer> rootReference = nullptr, const bool& enabled = true);
	FileParameter* addFileParameter(const juce::String& niceName, const juce::String& description, const juce::String& initialValue = "", const bool& enabled = true);

	Trigger* addTrigger(const juce::String& niceName, const juce::String& description, const bool& enabled = true, int index = -1);

	void addTriggerInternal(Trigger* t, int index = -1);
	void addParameterInternal(Parameter* p, int index = -1);

	juce::UndoableAction* removeUndoableControllable(Controllable* c, bool onlyReturnAction = false);
	void removeControllable(juce::WeakReference<Controllable> c, bool deleteObject = true);
	Controllable* getControllableByName(const juce::String& name, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);
	Parameter* getParameterByName(const juce::String& name, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);

	void addChildControllableContainer(ControllableContainer* container, bool owned = false, int index = -1, bool notify = true);
	void addChildControllableContainers(juce::Array<ControllableContainer*> containers, bool owned = false, int index = -1, bool notify = true);
	void removeChildControllableContainer(ControllableContainer* container);


	ControllableContainer* getControllableContainerByName(const juce::String& name, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);
	ControllableContainer* getControllableContainerForAddress(const juce::String& address, bool recursive = false, bool getNotExposed = false, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);
	ControllableContainer* getControllableContainerForAddress(juce::StringArray  addressSplit, bool recursive = false, bool getNotExposed = false, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);

	virtual void setParentContainer(ControllableContainer* container);
	void updateChildrenControlAddress();


	virtual juce::Array<juce::WeakReference<Controllable>> getAllControllables(bool recursive = false);
	virtual juce::Array<juce::WeakReference<Parameter>> getAllParameters(bool recursive = false);
	virtual juce::Array<juce::WeakReference<ControllableContainer>> getAllContainers(bool recursive = false);
	virtual Controllable* getControllableForAddress(const juce::String& address, bool recursive = true);
	virtual Controllable* getControllableForAddress(juce::StringArray addressSplit, bool recursive = true);
	bool containsControllable(Controllable* c, int maxSearchLevels = -1);
	juce::String getControlAddress(ControllableContainer* relativeTo = nullptr);


	//Remote Control
	virtual void handleAddFromRemoteControl(juce::var data) {}
	virtual void handleRemoveFromRemoteControl() {}
	virtual void handleLoadFromRemoteControl(juce::var data) { loadJSONData(data); }
	virtual juce::var handleSaveFromRemoteControl() { return getJSONData(); }


	void sortControllables();

	void dispatchFeedback(Controllable* c);
	void dispatchState(Controllable* c);

	virtual void controllableStateChanged(Controllable* c) override;
	virtual void parameterValueChanged(Parameter* p) override;
	virtual void parameterRangeChanged(Parameter* p) override;
	virtual void triggerTriggered(Trigger* p) override;

	void controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

	virtual void controllableNameChanged(Controllable* c, const juce::String &prevName) override;
	virtual void askForRemoveControllable(Controllable* c, bool addToUndo = false) override;

	void warningChanged(WarningTarget*);
	virtual juce::String getWarningMessage(const juce::String& id = WarningTarget::warningAllId) const override;
	virtual juce::String getWarningTargetName() const override;

	virtual juce::var getJSONData(bool includeNonOverriden = false);
	virtual void loadJSONData(juce::var data, bool createIfNotThere = false);
	virtual void loadJSONDataInternal(juce::var /*data*/) { /* to be overriden by child classes */ }
	virtual void afterLoadJSONDataInternal() {} //allow for calling methods after isCurrentlyLoadingData is set to false

	//Remote control
	virtual juce::var getRemoteControlData();
	virtual void getRemoteControlDataInternal(juce::var& /*data*/) {}
	virtual bool handleRemoteControlData(const juce::OSCMessage&, const juce::String & = juce::String()) { return false; }
	virtual bool handleRemoteControlData(Controllable*, const juce::OSCMessage& , const juce::String&  = juce::String()) { return false; }

	virtual void controllableContainerNameChanged(ControllableContainer*, const juce::String& prevName) override;
	virtual void childStructureChanged(ControllableContainer*)override;
	virtual void childAddressChanged(ControllableContainer*) override;

	bool isNameTaken(const juce::String& name, bool searchNiceName = true, Controllable* excludeC = nullptr, ControllableContainer* excludeCC = nullptr);
	juce::String getUniqueNameInContainer(const juce::String& sourceName, bool searchNiceName = true, int suffix = 0);

	//SCRIPT
	//virtual void updateScriptObjectInternal() override;
	static juce::var getChildFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getParentFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var setNameFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var setCollapsedFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var selectFromScript(const juce::var::NativeFunctionArgs& a);

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
	static juce::var addAutomationFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var addContainerFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var removeContainerFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var removeControllableFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var clearFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var getControlAddressFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getScriptControlAddressFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var getContainersFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var getControllablesFromScript(const juce::var::NativeFunctionArgs& args);

	static juce::var getJSONDataFromScript(const juce::var::NativeFunctionArgs& args);
	static juce::var loadJSONDataFromScript(const juce::var::NativeFunctionArgs& args);

	static bool checkNumArgs(const juce::String& logName, const juce::var::NativeFunctionArgs& args, int expectedArgs);

	juce::String getScriptTargetString() override;

	bool isAttachedToRoot();

protected:
	virtual void onContainerNiceNameChanged() {};
	virtual void onContainerShortNameChanged(const juce::String&) {};
	virtual void onContainerParameterChanged(Parameter*) {};
	virtual void onControllableStateChanged(Controllable*) {};
	virtual void onExternalParameterValueChanged(Parameter*) {}; //When listening to other child controllable than this container's children
	virtual void onExternalParameterRangeChanged(Parameter*) {};
	virtual void onControllableFeedbackUpdate(ControllableContainer*, Controllable*) {}
	virtual void onContainerTriggerTriggered(Trigger*) {};
	virtual void onExternalTriggerTriggered(Trigger*) {}; //When listening to other child controllable than this container's children
	virtual void onControllableAdded(Controllable*) {};
	virtual void onControllableRemoved(Controllable*) {};
	virtual void onContainerParameterChangedAsync(Parameter*, const juce::var& /*value*/) {};
	virtual void onWarningChanged(WarningTarget*) {};
	virtual void onChildContainerAdded(ControllableContainer*) {};
  virtual void onChildContainerRemoved(ControllableContainer*) {};

public:
	DECLARE_INSPECTACLE_SAFE_LISTENER(ControllableContainer, controllableContainer);

	QueuedNotifier<ContainerAsyncEvent> queuedNotifier;

	void addAsyncContainerListener(ContainerAsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedContainerListener(ContainerAsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncContainerListener(ContainerAsyncListener* listener) {
		if (isBeingDestroyed) return;
		queuedNotifier.removeListener(listener);
	}

	virtual void clear();



	juce::WeakReference<ControllableContainer>::Master masterReference;
	friend class juce::WeakReference<ControllableContainer>;

protected:
	virtual void notifyStructureChanged();
	void newMessage(const Parameter::ParameterEvent& e)override;
	void newMessage(const WarningTarget::WarningTargetEvent& e) override;


public:
	class ControllableContainerAction :
		public juce::UndoableAction
	{
	public:
		ControllableContainerAction(ControllableContainer* cc) :
			containerRef(cc)
		{
			controlAddress = cc->getControlAddress();
		}

		juce::WeakReference<ControllableContainer> containerRef;
		juce::String controlAddress;

		ControllableContainer* getControllableContainer();
	};

	class ControllableContainerChangeNameAction :
		public ControllableContainerAction
	{
	public:
		ControllableContainerChangeNameAction(ControllableContainer* cc, juce::String oldName, juce::String newName) :
			ControllableContainerAction(cc),
			oldName(oldName),
			newName(newName)
		{
		}

		juce::String oldName;
		juce::String newName;

		bool perform() override;
		bool undo() override;
	};

	class ControllableContainerControllableAction :
		public ControllableContainerAction
	{
	public:
		ControllableContainerControllableAction(ControllableContainer* cc, Controllable* c);


		juce::WeakReference<Inspectable> cRef;
		juce::String cShortName;
		juce::var data;
		juce::String cType;

		Controllable* getItem();
	};

	class AddControllableAction :
		public ControllableContainerControllableAction
	{
	public:
		AddControllableAction(ControllableContainer* cc, Controllable* c) :
			ControllableContainerControllableAction(cc, c)
		{
		}

		bool perform() override;
		bool undo() override;
	};

	class RemoveControllableAction :
		public ControllableContainerControllableAction
	{
	public:
		RemoveControllableAction(ControllableContainer* cc, Controllable* c);

		bool perform() override;
		bool undo() override;
	};



	virtual InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
	virtual DashboardItem* createDashboardItem() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableContainer)
};


class EnablingControllableContainer :
	public ControllableContainer
{
public:
	EnablingControllableContainer(const juce::String& n, bool canBeDisabled = true);
	BoolParameter* enabled;

	bool canBeDisabled;
	void setCanBeDisabled(bool value);

	virtual InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
};
