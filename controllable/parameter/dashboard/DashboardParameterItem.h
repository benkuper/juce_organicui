#pragma once


class DashboardParameterItem :
	public DashboardControllableItem
{
public:
	DashboardParameterItem(Parameter* parameter = nullptr);
	virtual ~DashboardParameterItem();

	juce::WeakReference<Parameter> parameter;
	
	BoolParameter* showValue;
	ColorParameter* bgColor;
	ColorParameter* fgColor;
	FileParameter* btImage;

	EnumParameter* style;
	virtual DashboardItemUI* createUI() override;

	InspectableEditor* getStyleEditor(bool isRoot, juce::Array<Inspectable *> inspectables);

	virtual void setInspectableInternal(Inspectable* i) override;
	void onExternalParameterValueChanged(Parameter* p) override;

	virtual void updateStyleOptions();

	virtual juce::var getServerData() override;

	static DashboardParameterItem* create(juce::var) { return new DashboardParameterItem(); }
	virtual juce::String getTypeString() const override { return "DashboardParameterItem"; }
};

class DashboardTargetParameterItem :
	public DashboardParameterItem
{
public:
	DashboardTargetParameterItem(TargetParameter* parameter = nullptr);
	virtual ~DashboardTargetParameterItem();

	BoolParameter* showFullAddress;
	BoolParameter* showParentName;
	IntParameter* parentLabelLevel;
	BoolParameter* showLearnButton;

	virtual juce::var getServerData() override;

	static DashboardParameterItem* create(juce::var) { return new DashboardTargetParameterItem(); }
	virtual juce::String getTypeString() const override { return "DashboardTargetParameterItem"; }
};

class DashboardEnumParameterItem :
	public DashboardParameterItem,
	public EnumParameter::Listener
{
public:
	DashboardEnumParameterItem(EnumParameter* parameter = nullptr);
	virtual ~DashboardEnumParameterItem();

	// Inherited via Listener
	virtual void enumOptionAdded(EnumParameter*, const juce::String&) override;
	virtual void enumOptionUpdated(EnumParameter*, int index, const juce::String& prevKey, const juce::String& newKey) override;
	virtual void enumOptionRemoved(EnumParameter*, const juce::String&) override;

	static DashboardParameterItem* create(juce::var) { return new DashboardEnumParameterItem(); }
	virtual juce::String getTypeString() const override { return "DashboardEnumParameterItem"; }
};




