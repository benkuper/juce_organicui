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
	std::function<ShapeShifterContent* (const juce::String&)> createFunc;
	juce::String attachToContentName;

	ShapeShifterDefinition(const juce::String& _contentName, std::function<ShapeShifterContent* (const juce::String&)> createFunc, const juce::String& _attachToContentName = "") :
		contentName(_contentName),
		createFunc(createFunc),
		attachToContentName(_attachToContentName)
	{
	}
};

class ShapeShifterFactory
{
public:
	juce_DeclareSingleton(ShapeShifterFactory, true);

	ShapeShifterFactory();
	~ShapeShifterFactory() {};
	juce::OwnedArray<ShapeShifterDefinition> defs;
	juce::StringPairArray aliases;

	static ShapeShifterContent* createContent(const juce::String& contentName)
	{
		const auto resolvedName = getInstance()->aliases.getValue(contentName, contentName);
		for (auto& d : getInstance()->defs)
		{
			if (d->contentName == resolvedName) return d->createFunc(resolvedName);
		}

		return nullptr;
	}

	static juce::String getAttachToContentName(const juce::String& contentName)
	{
		for (auto& d : getInstance()->defs)
		{
			if (d->contentName == contentName) return d->attachToContentName;
		}

		return "";
	}

	ShapeShifterDefinition* getDefinitionForContentName(const juce::String& contentName)
	{
		for (auto& d : defs)
		{
			if (d->contentName == contentName) return d;
		}

		return nullptr;
	}

};