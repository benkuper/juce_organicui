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

	Point2DParameter* canvasSize;
	FileParameter* bgImage;
	FloatParameter* bgImageScale;
	FloatParameter* bgImageAlpha;
};