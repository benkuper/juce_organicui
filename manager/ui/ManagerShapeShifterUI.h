/*
  ==============================================================================

	ManagerShapeShifterUI.h
	Created: 29 Oct 2016 5:22:21pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class M, class T, class G>
class NestingManagerShapeShifterUI :
	public ManagerUI<M, T, G>,
	public ShapeShifterContent
{
public:
	NestingManagerShapeShifterUI(const juce::String& contentName, M* _manager, bool useViewport = true) :
		ManagerUI<M, T, G>(contentName, _manager, useViewport),
		ShapeShifterContent(this, contentName)
	{

	}
};


template<class M, class T>
class ManagerShapeShifterUI :
	public NestingManagerShapeShifterUI<M, T, ItemGroup<M>>
{
public:
	ManagerShapeShifterUI(const juce::String& contentName, M* _manager, bool useViewport = true) :
		NestingManagerShapeShifterUI<M, T, ItemGroup<M>>(contentName, _manager, useViewport)
	{
	}
};


template<class M, class T, class G>
class NestingManagerShapeShifterViewUI :
	public ManagerViewUI<M, T, ItemGroup<G>>,
	public ShapeShifterContent
{
public:
	NestingManagerShapeShifterViewUI(const juce::String& contentName, M* _manager) :
		ManagerViewUI<M, T, G>(contentName, _manager),
		ShapeShifterContent(this, contentName)
	{

	}
};

template<class M, class T>
class ManagerShapeShifterViewUI :
	public NestingManagerShapeShifterViewUI<M, T, ItemGroup<M>>
{
	public:
	ManagerShapeShifterViewUI(const juce::String& contentName, M* _manager) :
		NestingManagerShapeShifterViewUI<M, T, ItemGroup<M>>(contentName, _manager)
	{
	}
};