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
	public Controllable::ControllableListener,
	public Inspectable::InspectableListener,
	public EngineListener
{
public:

	TargetParameter(const juce::String& niceName, const juce::String& description, const juce::String& initialValue, juce::WeakReference<ControllableContainer> rootReference = nullptr, bool enabled = true);
	~TargetParameter();

	enum TargetType { CONTAINER, CONTROLLABLE };
	TargetType targetType;


	bool useGhosting;
	juce::String ghostValue;

	bool showFullAddressInEditor;
	bool showParentNameInEditor;
	int maxDefaultSearchLevel;
	int defaultParentLabelLevel;
	bool isTryingFixingLink;
	bool manuallySettingNull;

	juce::StringArray typesFilter; //leave empty to allow all when not using custom functions
	juce::StringArray excludeTypesFilter; //leave empty to allow all when not using custom functions
	

	juce::WeakReference<ControllableContainer> rootContainer;

	juce::WeakReference<Controllable> target;
	juce::WeakReference<ControllableContainer> targetContainer;

	juce::WeakReference<Controllable> previousTarget; //temporary to keep the previous target when changing it
	juce::WeakReference<ControllableContainer> previousTargetContainer; //temporary to keep the previous target when changing it

	std::function<bool(Controllable*)> customTargetFilterFunc;
	std::function<void(const juce::StringArray&, const juce::StringArray&, ControllableContainer*, std::function<void(Controllable*)>)> customGetTargetFunc;
	std::function<juce::Array<TargetStepButton*>(Controllable*)> customGetControllableLabelFunc;
	std::function<bool(Controllable*)> customCheckAssignOnNextChangeFunc;

	std::function<bool(ControllableContainer*)> defaultContainerTypeCheckFunc;
	std::function<juce::Array<TargetStepButton*>(ControllableContainer*)> customGetContainerLabelFunc;
	std::function<void(ControllableContainer*, std::function<void(ControllableContainer*)>)> customGetTargetContainerFunc;

	void resetValue(bool silentSet = false) override;
	void setGhostValue(const juce::String& ghostVal);

	void setValueFromTarget(Controllable*, bool addToUndo = false);
	void setValueFromTarget(ControllableContainer*, bool addToUndo = false);

	void setValueInternal(juce::var&) override;

	juce::var getCroppedValue(juce::var val) override;

	Controllable* getTargetControllable();
	Trigger* getTargetTrigger();
	Parameter* getTargetParameter();
	ControllableContainer* getTargetContainer();
	template<class T>
	T* getTargetAs() { return dynamic_cast<T*>(target.get()); }
	template<class T>
	T* getTargetContainerAs() { return dynamic_cast<T*>(targetContainer.get()); }

	void setTarget(juce::WeakReference<Controllable>);
	void setTarget(juce::WeakReference<ControllableContainer>);

	void tryFixBrokenLink();

	void setRootContainer(juce::WeakReference<ControllableContainer> newRootContainer, bool engineIfNull = true, bool forceSetValue = true);

	void childStructureChanged(ControllableContainer*) override;

	void controllableControlAddressChanged(Controllable* c) override; //for target Controllable
	void childAddressChanged(ControllableContainer* cc) override;

	void inspectableDestroyed(Inspectable* i) override;

	bool setAttributeInternal(juce::String param, juce::var value) override;
	juce::var getAttributeInternal(juce::String name) const override;
	virtual juce::StringArray getValidAttributes() const override;

	juce::var getJSONDataInternal() override;
	void loadJSONDataInternal(juce::var data) override;

	void endLoadFile() override;

	TargetParameterUI* createTargetUI(juce::Array<TargetParameter*> parameters = {});
	ControllableUI* createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	virtual DashboardItem* createDashboardItem() override;


	static juce::var getTargetFromScript(const juce::var::NativeFunctionArgs& a);


	static StringParameter* create() { return new TargetParameter("New TargetParameter", "", ""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Target"; }

};
