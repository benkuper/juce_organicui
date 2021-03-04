
#pragma once

class DashboardCommentItem :
	public DashboardItem
{
public:
	DashboardCommentItem();
	virtual ~DashboardCommentItem();

	StringParameter* text;
	FloatParameter* size;
	ColorParameter* color;
	FloatParameter* bgAlpha;

	var getServerData() override;
	
	virtual DashboardItemUI* createUI() override;

	String getTypeString() const override { return "DashboardCommentItem"; }
	static DashboardCommentItem* create(var) { return new DashboardCommentItem(); }
};