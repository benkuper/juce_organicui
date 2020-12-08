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

	//override all to avoid sending events to the real parent container
	virtual void controllableStateChanged(Controllable*);
	virtual void controllableFeedbackStateChanged(Controllable*);
	virtual void controllableControlAddressChanged(Controllable* c) override;
	virtual void controllableNameChanged(Controllable*);
	virtual void askForRemoveControllable(Controllable*, bool /*addToUndo*/ = false);


	virtual var getServerData() override;

	virtual String getTypeString() const override { return "DashboardControllableItem"; }

	DECLARE_ASYNC_EVENT(DashboardControllableItem, DashboardControllableItem, dashboardItem, ENUM_LIST(NEEDS_UI_UPDATE))
};
