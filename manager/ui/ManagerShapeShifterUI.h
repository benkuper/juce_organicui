/*
  ==============================================================================

	ManagerShapeShifterUI.h
	Created: 29 Oct 2016 5:22:21pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class M, class T>
class ManagerShapeShifterUI :
	public ManagerUI<M, T>,
	public ShapeShifterContent
{
public:
	ManagerShapeShifterUI(const juce::String& contentName, M* _manager, bool useViewport = true) :
		ManagerUI<M, T>(contentName, _manager, useViewport),
		ShapeShifterContent(this, contentName)
	{

	}
};


template<class M, class T>
class ManagerShapeShifterViewUI :
	public ManagerViewUI<M, T>,
	public ShapeShifterContent
{
public:
	ManagerShapeShifterViewUI(const juce::String& contentName, M* _manager) :
		ManagerViewUI<M, T>(contentName, _manager),
		ShapeShifterContent(this, contentName)
	{

	}
};