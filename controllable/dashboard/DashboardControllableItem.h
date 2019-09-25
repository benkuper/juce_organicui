#pragma once


class DashboardControllableItem :
	public DashboardInspectableItem
{
public:
	DashboardControllableItem(Controllable* item = nullptr);
	~DashboardControllableItem();

	WeakReference<Controllable> controllable;

	BoolParameter * showLabel;
	ColorParameter* textColor;
	StringParameter* customLabel;
	StringParameter* customDescription;

	virtual var getJSONData();
	virtual void loadJSONDataItemInternal(var data) override;

	virtual void setInspectableInternal(Inspectable * i) override;


	virtual String getTypeString() const override { return "DashboardControllableItem"; }
};