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
	BoolParameter * opaqueBackground;
	ColorParameter* contourColor;
	FloatParameter* contourThickness;
	StringParameter* customLabel;
	StringParameter* customDescription;

    virtual var getJSONData() override;
	virtual void loadJSONDataItemInternal(var data) override;

	virtual void setInspectableInternal(Inspectable * i) override;

	virtual void ghostInspectable() override;
	virtual void checkGhost() override;

	virtual String getTypeString() const override { return "DashboardControllableItem"; }
};
