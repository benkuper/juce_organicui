
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

	juce::var getServerData() override;
	
	virtual juce::var getItemParameterFeedback(Parameter* p) override;

	static DashboardCommentItem* create(juce::var) { return new DashboardCommentItem(); }

	DECLARE_TYPE("DashboardCommentItem");
	DECLARE_UI_FUNC;
};