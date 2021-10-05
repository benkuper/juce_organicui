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
	enum Type { CUSTOM,TRIGGER,FLOAT,INT,BOOL,STRING,ENUM,POINT2D,POINT3D,TARGET,COLOR, TYPE_MAX };
	static const Array<String> typeNames;
	
	Controllable(const Type &type, const String &niceName, const String &description, bool enabled = true);
	virtual ~Controllable();

	Type type;
	String niceName;
	String shortName;
	String description;
	String argumentsDescription;

	bool enabled;
	bool canBeDisabledByUser;
	bool descriptionIsEditable;
	bool hasCustomShortName;
	bool isControllableFeedbackOnly;
	bool includeInScriptObject;

	String controlAddress;

	//save & load
	bool isSavable;
	bool saveValueOnly;
	bool isLoadingData;
	bool isControlledByParrot;

	//user control
	bool isCustomizableByUser;
	bool isRemovableByUser;
	bool userCanSetReadOnly;

	bool replaceSlashesInShortName;

	//dashboard
	int dashboardDefaultLabelParentLevel; //used to show another 
	bool dashboardDefaultAppendLabel;		//if default parent level is not 0 and this this will 
										   //decide if this controllable's label will be added at the end

	
	WeakReference<ControllableContainer> parentContainer;

	UndoableAction * setUndoableNiceName(const String &_niceName, bool onlyReturnAction = false);
	void setNiceName(const String &_niceName);
	void setCustomShortName(const String &_shortName);
	void setAutoShortName();

	virtual void setEnabled(bool value, bool silentSet = false, bool force = false);
	virtual void setControllableFeedbackOnly(bool value);

	void notifyStateChanged();

	void setParentContainer(ControllableContainer * container);

	template<class T>
	T * getParentAs() { 
		if (parentContainer == nullptr || parentContainer.wasObjectDeleted()) return nullptr;
		return dynamic_cast<T *>(parentContainer.get());
	}
	void updateControlAddress();

	void remove(bool addToUndo = false); // called from external to make this object ask for remove

	virtual void updateLiveScriptObjectInternal(DynamicObject * parent = nullptr) override;

	virtual bool shouldBeSaved();

	virtual var getJSONData(ControllableContainer * relativeTo = nullptr);
	virtual var getJSONDataInternal() { return var(new DynamicObject()); } // to be overriden
	virtual void loadJSONData(var data);
	virtual void loadJSONDataInternal(var data) {} //to be overriden

	virtual void setupFromJSONData(var data);

	String getControlAddress(ControllableContainer * relativeTo = nullptr);

	// used for generating editor
	virtual ControllableUI * createDefaultUI() = 0;

	virtual DashboardItem * createDashboardItem() override;
	virtual String getDefaultDashboardLabel() const;

	virtual void setAttribute(String param, var value);
	virtual StringArray getValidAttributes() const;

	static var setValueFromScript(const juce::var::NativeFunctionArgs& a);
	static var checkIsParameterFromScript(const juce::var::NativeFunctionArgs& a);
	static var getParentFromScript(const juce::var::NativeFunctionArgs& a);
	static var setNameFromScript(const juce::var::NativeFunctionArgs& a);
	static var setAttributeFromScript(const juce::var::NativeFunctionArgs& a);

	static var getControlAddressFromScript(const juce::var::NativeFunctionArgs& a);
	static var getScriptControlAddressFromScript(const juce::var::NativeFunctionArgs& a);


	String getScriptTargetString() override;
	
	virtual String getWarningTargetName() const override;
	
	virtual InspectableEditor * getEditorInternal(bool /*isRootEditor*/) override;
	virtual ControllableDetectiveWatcher* getDetectiveWatcher();
	virtual String getTypeString() const { jassert(false); return ""; } //should be overriden


	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void controllableStateChanged(Controllable *) {}
		virtual void controllableFeedbackStateChanged(Controllable *) {}
		virtual void controllableControlAddressChanged(Controllable *) {}
		virtual void controllableNameChanged(Controllable *) {}
		virtual void askForRemoveControllable(Controllable *, bool /*addToUndo*/ = false) {}
	};

	ListenerList<Listener> listeners;
	void addControllableListener(Listener* newListener) { listeners.add(newListener); }
	void removeControllableListener(Listener* listener) { listeners.remove(listener); }

																		// ASYNC
	class  ControllableEvent
	{
	public:
		enum Type { STATE_CHANGED, FEEDBACK_STATE_CHANGED, CONTROLADDRESS_CHANGED, NAME_CHANGED, CONTROLLABLE_REMOVED };

		ControllableEvent(Type t, Controllable * c) : type(t),controllable(c) {}

		Type type;
		Controllable * controllable;
	};

	QueuedNotifier<ControllableEvent> queuedNotifier;
	typedef QueuedNotifier<ControllableEvent>::Listener AsyncListener;


	void addAsyncControllableListener(AsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedControllableListener(AsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncControllableListener(AsyncListener* listener) { queuedNotifier.removeListener(listener); }


private:
	WeakReference<Controllable>::Master masterReference;
	friend class WeakReference<Controllable>;


public:
	class ControllableAction :
		public UndoableAction
	{
	public:
		ControllableAction(Controllable * c) :
			controllableRef(c)
		{
			controlAddress = c->getControlAddress();
		}

		WeakReference<Controllable> controllableRef;
		String controlAddress;

		Controllable * getControllable();
	};

	class ControllableChangeNameAction :
		public ControllableAction
	{
	public:
		ControllableChangeNameAction(Controllable * c, String oldName, String newName) :
			ControllableAction(c),
			oldName(oldName),
			newName(newName)
		{
		}

		String oldName;
		String newName;

		bool perform() override;
		bool undo() override;
	};

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controllable)
};
