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
	public Controllable::Listener,
	public Parameter::AsyncListener,
	public Trigger::Listener,
	public ControllableContainerListener,
	public Inspectable,
	public ScriptTarget,
	public DashboardItemTarget,
	public WarningTarget::AsyncListener

{
public:
	ControllableContainer(const String &niceName);
	virtual ~ControllableContainer();

	String niceName;
	String shortName;
	bool hasCustomShortName;
	bool allowSameChildrenNiceNames;

	bool nameCanBeChangedByUser;

	//Editor
	bool canInspectChildContainers;
	bool editorIsCollapsed;
	bool editorCanBeCollapsed;
	bool hideEditorHeader;
	bool skipLabelInTarget;
	bool userCanAddControllables;

	std::function<void(ControllableContainer *)> customUserCreateControllableFunc; 
	StringArray userAddControllablesFilters;
	

	std::function<InspectableEditor *(ControllableContainer *, bool)> customGetEditorFunc;

	//save / load
	bool saveAndLoadRecursiveData;
	bool saveAndLoadName;
	bool includeInRecursiveSave;
	bool includeTriggersInSaveLoad;
	bool isCurrentlyLoadingData;
	bool notifyStructureChangeWhenLoadingData;
	bool canBeCopiedAndPasted;

	//Script
	bool includeInScriptObject;

	static ControllableComparator comparator;

	OwnedArray<Controllable, CriticalSection> controllables;
	Array<WeakReference<ControllableContainer>, CriticalSection> controllableContainers;
	OwnedArray<ControllableContainer, CriticalSection> ownedContainers;
	WeakReference<ControllableContainer> parentContainer;

	UndoableAction * setUndoableNiceName(const String &_niceName, bool onlyReturnAction = false);
	void setNiceName(const String &_niceName);
	void setCustomShortName(const String &_shortName);
	void setAutoShortName();


	UndoableAction * addUndoableControllable(Controllable * c, bool onlyReturnAction = false);
	void addControllable(Controllable * c);
	void addParameter(Parameter * p);
	FloatParameter * addFloatParameter(const String &niceName, const String &description, const float &initialValue, const float &minValue = INT32_MIN, const float &maxValue = INT32_MAX, const bool &enabled = true);
	IntParameter * addIntParameter(const String &niceName, const String &description, const int &initialValue, const int &minValue = INT32_MIN, const int &maxValue = INT32_MAX, const bool &enabled = true);
	BoolParameter * addBoolParameter(const String &niceName, const String &description, const bool &value, const bool &enabled = true);
	StringParameter * addStringParameter(const String &niceName, const String &description, const String &value, const bool &enabled = true);
	EnumParameter * addEnumParameter(const String &niceName, const String &description, const bool &enabled = true);
	Point2DParameter * addPoint2DParameter(const String &niceName, const String &description, const bool &enabled = true);
	Point3DParameter * addPoint3DParameter(const String &niceName, const String &description, const bool &enabled = true);
	ColorParameter * addColorParameter(const String &niceName, const String &description, const Colour &initialColor, const bool &enabled = true);
	TargetParameter * addTargetParameter(const String &niceName, const String &description, WeakReference<ControllableContainer> rootReference = nullptr, const bool &enabled = true);
	FileParameter * addFileParameter(const String &niceName, const String &description, const String &initialValue = "");

	Trigger * addTrigger(const String &niceName, const String &description, const bool &enabled = true);

	void addTriggerInternal(Trigger * t);
	void addParameterInternal(Parameter * p);

	UndoableAction * removeUndoableControllable(Controllable * c, bool onlyReturnAction = false);
	void removeControllable(WeakReference<Controllable> c);
	Controllable * getControllableByName(const String &name, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);
	Parameter * getParameterByName(const String &name, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);

	void addChildControllableContainer(ControllableContainer* container, bool owned = false, int index = -1, bool notify = true);
	void addChildControllableContainers(Array<ControllableContainer *> containers, bool owned = false, int index = -1, bool notify = true);
	void removeChildControllableContainer(ControllableContainer *container);
	

	ControllableContainer * getControllableContainerByName(const String &name, bool searchNiceNameToo = false, bool searchLowerCaseToo = true);
	ControllableContainer * getControllableContainerForAddress(const String &address, bool recursive = false, bool getNotExposed = false);
	ControllableContainer * getControllableContainerForAddress(StringArray  addressSplit, bool recursive = false, bool getNotExposed = false);

	void setParentContainer(ControllableContainer * container);
	void updateChildrenControlAddress();


	virtual Array<WeakReference<Controllable>> getAllControllables(bool recursive = false, bool getNotExposed = false);
	virtual Array<WeakReference<Parameter>> getAllParameters(bool recursive = false, bool getNotExposed = false);
	virtual Array<WeakReference<ControllableContainer>> getAllContainers(bool recursive = false);
	virtual Controllable * getControllableForAddress(const String &address, bool recursive = true, bool getNotExposed = false);
	virtual Controllable * getControllableForAddress(StringArray addressSplit, bool recursive = true, bool getNotExposed = false);
	bool containsControllable(Controllable * c, int maxSearchLevels = -1);
	String getControlAddress(ControllableContainer * relativeTo = nullptr);

	void orderControllablesAlphabetically();

	void dispatchFeedback(Controllable* c);
	void dispatchState(Controllable * c);

	virtual void controllableStateChanged(Controllable* c) override;
	virtual void parameterValueChanged(Parameter * p) override;
	virtual void parameterRangeChanged(Parameter * p) override;
	virtual void triggerTriggered(Trigger * p) override;

	void controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;

	virtual void controllableNameChanged(Controllable * c) override;
	virtual void askForRemoveControllable(Controllable * c, bool addToUndo = false) override;

	void warningChanged(WarningTarget*);
	virtual String getWarningMessage() const override;
	virtual String getWarningTargetName() const override;

	virtual var getJSONData();
	virtual void loadJSONData(var data, bool createIfNotThere = false);
	virtual void loadJSONDataInternal(var /*data*/) { /* to be overriden by child classes */ }
	virtual void afterLoadJSONDataInternal() {} //allow for calling methods after isCurrentlyLoadingData is set to false

	virtual void controllableContainerNameChanged(ControllableContainer *) override;
	virtual void childStructureChanged(ControllableContainer *)override;
	virtual void childAddressChanged(ControllableContainer *) override;
	
	bool isNameTaken(const String& name, bool searchNiceName = true, Controllable* excludeC = nullptr, ControllableContainer* excludeCC = nullptr);
	String getUniqueNameInContainer(const String &sourceName, bool searchNiceName = true, int suffix = 0);

	//SCRIPT
	virtual void updateLiveScriptObjectInternal(DynamicObject * parent = nullptr) override;
	static var getChildFromScript(const var::NativeFunctionArgs &a);
	static var getParentFromScript(const juce::var::NativeFunctionArgs& a);
	static var setNameFromScript(const juce::var::NativeFunctionArgs& a);
	static var setCollapsedFromScript(const juce::var::NativeFunctionArgs& a);
	
	static var addTriggerFromScript(const var::NativeFunctionArgs &args);
	static var addBoolParameterFromScript(const var::NativeFunctionArgs &args);
	static var addIntParameterFromScript(const var::NativeFunctionArgs &args);
	static var addFloatParameterFromScript(const var::NativeFunctionArgs &args);
	static var addStringParameterFromScript(const var::NativeFunctionArgs &args);
	static var addEnumParameterFromScript(const var::NativeFunctionArgs &args);
	static var addTargetParameterFromScript(const var::NativeFunctionArgs &args);
	static var addColorParameterFromScript(const var::NativeFunctionArgs &args);
	static var addPoint2DParameterFromScript(const var::NativeFunctionArgs &args);
	static var addPoint3DParameterFromScript(const var::NativeFunctionArgs &args);
	static var addFileParameterFromScript(const var::NativeFunctionArgs &args);
	
	static var addContainerFromScript(const var::NativeFunctionArgs &args);
	static var removeContainerFromScript(const var::NativeFunctionArgs &args);
	static var removeControllableFromScript(const var::NativeFunctionArgs& args);

	static var getControlAddressFromScript(const var::NativeFunctionArgs& args);
	static var getScriptControlAddressFromScript(const var::NativeFunctionArgs &args);
	
	static bool checkNumArgs(const String &logName, const var::NativeFunctionArgs &args, int expectedArgs);
	
	String getScriptTargetString() override;

	// Inherited via DashboardItemTarget

protected:
	virtual void onContainerNiceNameChanged() {};
	virtual void onContainerShortNameChanged() {};
	virtual void onContainerParameterChanged(Parameter*) {};
	virtual void onControllableStateChanged(Controllable *) {};
	virtual void onExternalParameterValueChanged(Parameter *) {}; //When listening to other child controllable than this container's children
	virtual void onExternalParameterRangeChanged(Parameter *) {};
	virtual void onControllableFeedbackUpdate(ControllableContainer*, Controllable*) {}
	virtual void onContainerTriggerTriggered(Trigger *) {};
	virtual void onExternalTriggerTriggered(Trigger *) {}; //When listening to other child controllable than this container's children
	virtual void onControllableAdded(Controllable *) {}; 
	virtual void onControllableRemoved(Controllable *) {};
    virtual void onContainerParameterChangedAsync(Parameter *,const var & /*value*/){};
	virtual void onWarningChanged(WarningTarget*) {}

public:
    ListenerList<ControllableContainerListener> controllableContainerListeners;
    void addControllableContainerListener(ControllableContainerListener* newListener) { controllableContainerListeners.add(newListener);}
    void removeControllableContainerListener(ControllableContainerListener* listener) { controllableContainerListeners.remove(listener);}
    
	QueuedNotifier<ContainerAsyncEvent> queuedNotifier;
	
	void addAsyncContainerListener(ContainerAsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedContainerListener(ContainerAsyncListener * newListener) {queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncContainerListener(ContainerAsyncListener* listener) { queuedNotifier.removeListener(listener); }
	
	virtual void clear();



	WeakReference<ControllableContainer>::Master masterReference;
	friend class WeakReference<ControllableContainer>;

protected:
	virtual void notifyStructureChanged();
	void newMessage(const Parameter::ParameterEvent &e)override;
	void newMessage(const WarningTarget::WarningTargetEvent& e) override;


public:
	class ControllableContainerAction :
		public UndoableAction
	{
	public:
		ControllableContainerAction(ControllableContainer * cc) :
			containerRef(cc)
		{
			controlAddress = cc->getControlAddress();
		}

		WeakReference<ControllableContainer> containerRef;
		String controlAddress;

		ControllableContainer * getControllableContainer();
	};

	class ControllableContainerChangeNameAction :
		public ControllableContainerAction 
	{
	public:
		ControllableContainerChangeNameAction(ControllableContainer * cc, String oldName, String newName) :
			ControllableContainerAction(cc),
			oldName(oldName),
			newName(newName)
		{
		}

		String oldName;
		String newName;

		bool perform() override;
		bool undo() override;
	};

	class ControllableContainerControllableAction :
		public ControllableContainerAction
	{
	public:
		ControllableContainerControllableAction(ControllableContainer * cc, Controllable * c);

		
		WeakReference<Inspectable> cRef;
		String cShortName;
		var data; 
		String cType;

		Controllable * getItem();
	};

	class AddControllableAction :
		public ControllableContainerControllableAction
	{
	public:
		AddControllableAction(ControllableContainer * cc, Controllable * c) :
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
		RemoveControllableAction(ControllableContainer * cc, Controllable * c);

		bool perform() override;
		bool undo() override;
	};



	virtual InspectableEditor * getEditor(bool /*isRootEditor*/) override;
	virtual DashboardItem * createDashboardItem() override;

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableContainer)

			
};


class EnablingControllableContainer :
	public ControllableContainer
{
public:
	EnablingControllableContainer(const String &n, bool canBeDisabled = true);
	BoolParameter * enabled;

	bool canBeDisabled;
	void setCanBeDisabled(bool value);

	virtual InspectableEditor * getEditor(bool isRoot) override;
};
