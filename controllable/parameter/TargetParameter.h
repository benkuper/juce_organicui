/*
  ==============================================================================

	TargetParameter.h
	Created: 2 Nov 2016 5:00:04pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class TargetParameterUI;
class TargetStepButton;
class Trigger;

class TargetParameter :
	public StringParameter,
	public ControllableContainerListener,
	public Controllable::Listener,
	public Inspectable::InspectableListener
{
public:

	TargetParameter(const String& niceName, const String& description, const String& initialValue, WeakReference<ControllableContainer> rootReference = nullptr, bool enabled = true);
	~TargetParameter();

	enum TargetType { CONTAINER, CONTROLLABLE };
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
	std::function<void(const StringArray&, const StringArray&, ControllableContainer*, std::function<void(Controllable*)>)> customGetTargetFunc;
	std::function<Array<TargetStepButton*>(Controllable*)> customGetControllableLabelFunc;
	std::function<bool(Controllable*)> customCheckAssignOnNextChangeFunc;

	std::function<bool(ControllableContainer*)> defaultContainerTypeCheckFunc;
	std::function<Array<TargetStepButton*>(ControllableContainer*)> customGetContainerLabelFunc;
	std::function<void(ControllableContainer*, std::function<void(ControllableContainer*)>)> customGetTargetContainerFunc;

	void resetValue(bool silentSet = false) override;
	void setGhostValue(const String& ghostVal);

	void setValueFromTarget(Controllable*, bool addToUndo = false);
	void setValueFromTarget(ControllableContainer*, bool addToUndo = false);

	void setValueInternal(var&) override;

	var getCroppedValue(var val) override;

	Controllable* getTargetControllable();
	Trigger* getTargetTrigger();
	Parameter* getTargetParameter();
	ControllableContainer* getTargetContainer();
	template<class T>
	T* getTargetAs() { return dynamic_cast<T*>(target.get()); }
	template<class T>
	T* getTargetContainerAs() { return dynamic_cast<T*>(targetContainer.get()); }

	void setTarget(WeakReference<Controllable>);
	void setTarget(WeakReference<ControllableContainer>);

	void setRootContainer(WeakReference<ControllableContainer> newRootContainer, bool engineIfNull = true, bool forceSetValue = true);

	void childStructureChanged(ControllableContainer*) override;

	void controllableControlAddressChanged(Controllable* c) override; //for target Controllable
	void childAddressChanged(ControllableContainer* cc) override;

	void inspectableDestroyed(Inspectable* i) override;

	bool setAttributeInternal(String param, var value) override;
	virtual StringArray getValidAttributes() const override;

	var getJSONDataInternal() override;
	void loadJSONDataInternal(var data) override;

	TargetParameterUI* createTargetUI(Array<TargetParameter*> parameters = {});
	ControllableUI* createDefaultUI(Array<Controllable*> controllables = {}) override;

	virtual DashboardItem* createDashboardItem() override;


	static var getTargetFromScript(const juce::var::NativeFunctionArgs& a);


	static StringParameter* create() { return new TargetParameter("New TargetParameter", "", ""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Target"; }

};
