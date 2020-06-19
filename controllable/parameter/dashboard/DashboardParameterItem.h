#pragma once


class DashboardParameterItem :
	public DashboardControllableItem
{
public:
	DashboardParameterItem(Parameter* parameter = nullptr);
	~DashboardParameterItem();

	Parameter * parameter;
	
	BoolParameter* showValue;
	ColorParameter* bgColor;
	ColorParameter* fgColor;
	FileParameter* btImage;

	EnumParameter* style;

	virtual DashboardItemUI* createUI() override;

	virtual void setInspectableInternal(Inspectable* i) override;

	void onExternalParameterValueChanged(Parameter* p) override;

	var getServerData() override;

	static DashboardParameterItem* create(var) { return new DashboardParameterItem(); }
	virtual String getTypeString() const override { return "DashboardParameterItem"; }

};