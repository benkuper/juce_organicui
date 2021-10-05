#pragma once


class DashboardParameterItem :
	public DashboardControllableItem
{
public:
	DashboardParameterItem(Parameter* parameter = nullptr);
	~DashboardParameterItem();

	WeakReference<Parameter> parameter;
	
	BoolParameter* showValue;
	ColorParameter* bgColor;
	ColorParameter* fgColor;
	FileParameter* btImage;

	EnumParameter* style;
	//std::unique_ptr<ColorStyleManager> colorManager;

	virtual DashboardItemUI* createUI() override;

	virtual void setInspectableInternal(Inspectable* i) override;

	void onContainerParameterChangedInternal(Parameter* p) override;
	void onExternalParameterValueChanged(Parameter* p) override;

	var getServerData() override;

	static DashboardParameterItem* create(var) { return new DashboardParameterItem(); }
	virtual String getTypeString() const override { return "DashboardParameterItem"; }
};

