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
	virtual DashboardItemUI* createUI() override;

	InspectableEditor* getStyleEditor(Inspectable* c, bool isRoot);

	virtual void setInspectableInternal(Inspectable* i) override;
	void onExternalParameterValueChanged(Parameter* p) override;

	virtual void updateStyleOptions();

	var getServerData() override;

	static DashboardParameterItem* create(var) { return new DashboardParameterItem(); }
	virtual String getTypeString() const override { return "DashboardParameterItem"; }
};