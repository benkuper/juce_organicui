#pragma once


class DashboardParameterItem :
	public DashboardControllableItem
{
public:
	DashboardParameterItem(Parameter* parameter = nullptr);
	virtual ~DashboardParameterItem();

	WeakReference<Parameter> parameter;
	
	BoolParameter* showValue;
	ColorParameter* bgColor;
	ColorParameter* fgColor;
	FileParameter* btImage;

	EnumParameter* style;
	virtual DashboardItemUI* createUI() override;

	InspectableEditor* getStyleEditor(Inspectable* c, bool isRoot);

	virtual void setInspectableInternal(Inspectable* i) override;
	void onExternalParameterValueChanged(Parameter* p) override;

	virtual void updateStyleOptions();

	virtual var getServerData() override;

	static DashboardParameterItem* create(var) { return new DashboardParameterItem(); }
	virtual String getTypeString() const override { return "DashboardParameterItem"; }
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

	virtual var getServerData() override;

	static DashboardParameterItem* create(var) { return new DashboardTargetParameterItem(); }
	virtual String getTypeString() const override { return "DashboardTargetParameterItem"; }
};

