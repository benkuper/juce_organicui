/*
  ==============================================================================

    BaseManagerShapeShifterUI.h
    Created: 29 Oct 2016 5:22:21pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class M, class T, class U>
class BaseManagerShapeShifterUI :
	public BaseManagerUI<M, T, U>,
	public ShapeShifterContent
{
public:
	BaseManagerShapeShifterUI(const juce::String &contentName, M * _manager, bool useViewport = true) :
		BaseManagerUI<M, T, U>(contentName, _manager, useViewport),
		ShapeShifterContent(this,contentName)
	{

	}
};

template<class M, class T, class U>
class BaseManagerShapeShifterViewUI :
	public BaseManagerViewUI<M, T, U>,
	public ShapeShifterContent
{
public:
	BaseManagerShapeShifterViewUI(const juce::String &contentName, M * _manager) :
		BaseManagerViewUI<M, T, U>(contentName, _manager),
		ShapeShifterContent(this, contentName)
	{

	}
};