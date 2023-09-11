/*
  ==============================================================================

    ShapeShifterFactory.h
    Created: 18 May 2016 11:33:09am
    Author:  bkupe

  ==============================================================================
*/

#pragma once


class ShapeShifterDefinition
{
public:
	juce::String contentName;
	std::function<ShapeShifterContent*(const juce::String &)> createFunc;

	ShapeShifterDefinition(const juce::String &_contentName, std::function<ShapeShifterContent*(const juce::String &)> createFunc) :
		contentName(_contentName),
		createFunc(createFunc)
	{
	}
};

class ShapeShifterFactory
{
public:
	juce_DeclareSingleton(ShapeShifterFactory, true);

	ShapeShifterFactory();
	~ShapeShifterFactory(){};
	juce::OwnedArray<ShapeShifterDefinition> defs;

	static ShapeShifterContent * createContent(const juce::String &contentName)
	{
		for (auto &d : getInstance()->defs)
		{
			if (d->contentName == contentName) return d->createFunc(contentName);
		}

		return nullptr;
	}

};