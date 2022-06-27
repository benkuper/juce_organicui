/*
  ==============================================================================

    TargetParameter.h
    Created: 2 Nov 2016 5:00:04pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class TargetParameterUI;

class TargetParameter :
	public StringParameter,
	public ControllableContainerListener,
	public Controllable::Listener,
	public Inspectable::InspectableListener
{
public:
	
	TargetParameter(const String &niceName, const String &description, const String &initialValue,  WeakReference<ControllableContainer> rootReference = nullptr, bool enabled = true);
	~TargetParameter();

	enum TargetType {CONTAINER, CONTROLLABLE};
	TargetType targetType;


	bool useGhosting;
	String ghostValue;

	bool showFullAddressInEditor;
	bool showParentNameInEditor;
	int maxDefaultSearchLevel;
	int defaultParentLabelLevel;

	StringArray typesFilter; //leave empty to allow all when not using custom functions
	StringArray excludeTypesFilter; //leave empty to allow all when not using custom functions

	WeakReference<ControllableContainer> rootContainer;
	
	WeakReference<Controllable> target;
	WeakReference<ControllableContainer> targetContainer;
	
	std::function<bool(Controllable*)> customTargetFilterFunc;
	std::function<void(const StringArray &, const StringArray &, ControllableContainer*, std::function<void(Controllable *)>)> customGetTargetFunc;
	std::function<String(Controllable*)> customGetControllableLabelFunc;
	std::function<bool(Controllable*)> customCheckAssignOnNextChangeFunc;

	std::function<bool(ControllableContainer *)> defaultContainerTypeCheckFunc;
	std::function<String(ControllableContainer*)> customGetContainerLabelFunc;
	std::function<void(ControllableContainer*, std::function<void(ControllableContainer*)>)> customGetTargetContainerFunc;

	void resetValue(bool silentSet = false) override;
	void setGhostValue(const String &ghostVal);

	void setValueFromTarget(Controllable *, bool addToUndo = false);
	void setValueFromTarget(ControllableContainer *, bool addToUndo = false);

	void setValueInternal(var &) override;

	var getCroppedValue(var val) override;

	void setTarget(WeakReference<Controllable>);
	void setTarget(WeakReference<ControllableContainer>);

	void setRootContainer(WeakReference<ControllableContainer> newRootContainer);

	void childStructureChanged(ControllableContainer *) override;

	void controllableControlAddressChanged(Controllable* c) override; //for target Controllable
	void childAddressChanged(ControllableContainer* cc) override;

	void inspectableDestroyed(Inspectable * i) override;

	void setAttribute(String param, var value) override;
	virtual StringArray getValidAttributes() const override;

	var getJSONDataInternal() override;
	void loadJSONDataInternal(var data) override;

	TargetParameterUI* createTargetUI(Array<TargetParameter*> parameters = {});
	ControllableUI* createDefaultUI(Array<Controllable*> controllables = {}) override;

	virtual DashboardItem* createDashboardItem() override;


	static var getTargetFromScript(const juce::var::NativeFunctionArgs& a);


	static StringParameter * create() { return new TargetParameter("New TargetParameter", "", ""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Target"; }

};
