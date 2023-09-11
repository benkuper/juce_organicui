
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
	
	virtual DashboardItemUI* createUI() override;
	virtual juce::var getItemParameterFeedback(Parameter* p) override;

	juce::String getTypeString() const override { return "DashboardCommentItem"; }
	static DashboardCommentItem* create(juce::var) { return new DashboardCommentItem(); }
};