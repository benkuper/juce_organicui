
#pragma once

class DashboardCommentItem :
	public DashboardItem
{
public:
	DashboardCommentItem();
	virtual ~DashboardCommentItem();

	StringParameter* text;
	FloatParameter* size;
	FloatParameter* bgAlpha;

	var getServerData() override;
	
	virtual DashboardItemUI* createUI() override;
	virtual var getItemParameterFeedback(Parameter* p) override;

	String getTypeString() const override { return "DashboardCommentItem"; }
	static DashboardCommentItem* create(var) { return new DashboardCommentItem(); }
};