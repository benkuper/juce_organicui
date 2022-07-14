/*
  ==============================================================================

    DashboardItemManager.h
    Created: 19 Apr 2017 11:06:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemManager :
	public BaseManager<DashboardItem>,
	public DashboardFeedbackBroadcaster
{
public:
	DashboardItemManager();
	~DashboardItemManager();

	Point2DParameter* canvasSize;
	ColorParameter * bgColor;
	FileParameter* bgImage;
	FloatParameter* bgImageScale;
	FloatParameter* bgImageAlpha;

	void clear() override;
	void fillServerData(var & data);

	void onContainerParameterChanged(Parameter* p) override;
};