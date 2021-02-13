/*
  ==============================================================================

    DashboardItemManager.h
    Created: 19 Apr 2017 11:06:56pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemManager :
	public BaseManager<DashboardItem>
{
public:
	DashboardItemManager();
	~DashboardItemManager();

	CommentManager commentManager;

	Point2DParameter* canvasSize;
	FileParameter* bgImage;
	FloatParameter* bgImageScale;
	FloatParameter* bgImageAlpha;

	void clear() override;

	var getJSONData() override;
	void loadJSONDataManagerInternal(var data) override;
};