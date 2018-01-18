/*
  ==============================================================================

    ControllableContainer.h
    Created: 8 Mar 2016 1:15:36pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLABLECONTAINER_H_INCLUDED
#define CONTROLLABLECONTAINER_H_INCLUDED

class ControllableContainer :
	public Parameter::Listener,
	public Controllable::Listener,
	public Parameter::AsyncListener,
	public Trigger::Listener,
	public ControllableContainerListener,
	public Inspectable,
	public ScriptTarget

{
public:
	ControllableContainer(const String &niceName);
	virtual ~ControllableContainer();

	String niceName;
	String shortName;
	bool hasCustomShortName;

	bool skipControllableNameInAddress;
	
	bool nameCanBeChangedByUser;
	bool isTargettable; //for controllableChooser

	//Editor
	bool hideInEditor;
	bool canInspectChildContainers;
	bool editorIsCollapsed;
	bool editorCanBeCollapsed;
	bool hideEditorHeader;

	//save / load
	bool saveAndLoadRecursiveData;
	bool saveAndLoadName;
	bool includeTriggersInSaveLoad;

	//Script
	bool includeInScriptObject;

	static ControllableComparator comparator;

	Uuid uid;

	OwnedArray<Controllable, CriticalSection> controllables;
	Array<WeakReference<ControllableContainer>  > controllableContainers;
	WeakReference<ControllableContainer> parentContainer;

	UndoableAction * setUndoableNiceName(const String &_niceName, bool onlyReturnAction = false);
	void setNiceName(const String &_niceName);
	void setCustomShortName(const String &_shortName);
	void setAutoShortName();


	void addControllable(Controllable * p);
	void addParameter(Parameter * p);
	FloatParameter * addFloatParameter(const String &niceName, const String &description, const float &initialValue, const float &minValue = 0, const float &maxValue = 1, const bool &enabled = true);
	IntParameter * addIntParameter(const String &niceName, const String &description, const int &initialValue, const int &minValue, const int &maxValue, const bool &enabled = true);
	BoolParameter * addBoolParameter(const String &niceName, const String &description, const bool &value, const bool &enabled = true);
	StringParameter * addStringParameter(const String &niceName, const String &description, const String &value, const bool &enabled = true);
	EnumParameter * addEnumParameter(const String &niceName, const String &description, const bool &enabled = true);
	Point2DParameter * addPoint2DParameter(const String &niceName, const String &description, const bool &enabled = true);
	Point3DParameter * addPoint3DParameter(const String &niceName, const String &description, const bool &enabled = true);
	TargetParameter * addTargetParameter(const String &niceName, const String &description, WeakReference<ControllableContainer> rootReference = nullptr, const bool &enabled = true);
	Trigger * addTrigger(const String &niceName, const String &description, const bool &enabled = true);

	void addTriggerInternal(Trigger * t);
	void addParameterInternal(Parameter * p);


	void removeControllable(Controllable * c);
	Controllable * getControllableByName(const String &name, bool searchNiceNameToo = false);

	void addChildControllableContainer(ControllableContainer * container);
	void removeChildControllableContainer(ControllableContainer *container);
	

	ControllableContainer * getControllableContainerByName(const String &name, bool searchNiceNameToo = false);
	ControllableContainer * getControllableContainerForAddress(const String &address, bool recursive = false, bool getNotExposed = false);
	ControllableContainer * getControllableContainerForAddress(StringArray  addressSplit, bool recursive = false, bool getNotExposed = false);

	void setParentContainer(ControllableContainer * container);
	void updateChildrenControlAddress();


	virtual Array<WeakReference<Controllable>> getAllControllables(bool recursive = false, bool getNotExposed = false);
	virtual Array<WeakReference<Parameter>> getAllParameters(bool recursive = false, bool getNotExposed = false);
	virtual Controllable * getControllableForAddress(const String &address, bool recursive = true, bool getNotExposed = false);
	virtual Controllable * getControllableForAddress(StringArray addressSplit, bool recursive = true, bool getNotExposed = false);
	bool containsControllable(Controllable * c, int maxSearchLevels = -1);
	String getControlAddress(ControllableContainer * relativeTo = nullptr);

	void orderControllablesAlphabetically();

	void dispatchFeedback(Controllable * c);

	virtual void parameterValueChanged(Parameter * p) override;
	virtual void triggerTriggered(Trigger * p) override;

	void controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;
	virtual void onControllableFeedbackUpdate(ControllableContainer * , Controllable *) {}

	virtual void askForRemoveControllable(Controllable * c) override;

	virtual var getJSONData();
	virtual void loadJSONData(var data, bool createIfNotThere = false);
	virtual void loadJSONDataInternal(var /*data*/) { /* to be overriden by child classes */ }

	virtual void childStructureChanged(ControllableContainer *)override;
	virtual void childAddressChanged(ControllableContainer *) override;
	void parameterRangeChanged(Parameter * p) override;

	String getUniqueNameInContainer(const String &sourceName, int suffix = 0);

	//SCRIPT
	virtual void updateLiveScriptObjectInternal(DynamicObject * parent = nullptr) override;
	static var getChildFromScript(const var::NativeFunctionArgs &a);

private:
	// internal callback that a controllableContainer can override to react to any of it's parameter change
	//@ ben this is to avoid either:
	//      adding controllableContainerListener for each implementation
	//      or overriding parameterValueChanged and needing to call ControllableContainer::parameterValueChanged in implementation (it should stay independent as a different mechanism)
	//      or using dispatch feedback that triggers only exposedParams

	virtual void onContainerNiceNameChanged() {};
	virtual void onContainerShortNameChanged() {};
	virtual void onContainerParameterChanged(Parameter *) {};
	virtual void onExternalParameterChanged(Parameter *) {}; //When listening to other child controllable then this container's children
	virtual void onContainerTriggerTriggered(Trigger *) {};
	virtual void onExternalTriggerTriggered(Trigger *) {}; //When listening to other child controllable then this container's children

    virtual void onContainerParameterChangedAsync(Parameter *,const var & /*value*/){};
    

public:
    ListenerList<ControllableContainerListener> controllableContainerListeners;
    void addControllableContainerListener(ControllableContainerListener* newListener) { controllableContainerListeners.add(newListener);}
    void removeControllableContainerListener(ControllableContainerListener* listener) { controllableContainerListeners.remove(listener);}
    
	QueuedNotifier<ContainerAsyncEvent> queuedNotifier;
	typedef QueuedNotifier<ContainerAsyncEvent>::Listener ContainerAsyncListener;

	void addAsyncContainerListener(ContainerAsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedContainerListener(ContainerAsyncListener * newListener) {queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncContainerListener(ContainerAsyncListener* listener) { queuedNotifier.removeListener(listener); }
	
	void clear();

protected:
	void notifyStructureChanged();
	void newMessage(const Parameter::ParameterEvent &e)override;

	WeakReference<ControllableContainer>::Master masterReference;
	friend class WeakReference<ControllableContainer>;

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

	virtual InspectableEditor * getEditor(bool /*isRootEditor*/) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableContainer)
};


class EnablingControllableContainer :
	public ControllableContainer
{
public:
	EnablingControllableContainer(const String &n, bool canBeDisabled = true);
	BoolParameter * enabled;

	bool canBeDisabled;

	virtual InspectableEditor * getEditor(bool isRoot) override;
};

#endif  // CONTROLLABLECONTAINER_H_INCLUDED
