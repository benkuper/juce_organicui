/*
  ==============================================================================

    ShapeShifterFactory.h
    Created: 18 May 2016 11:33:09am
    Author:  bkupe

  ==============================================================================
*/

#ifndef SHAPESHIFTERFACTORY_H_INCLUDED
#define SHAPESHIFTERFACTORY_H_INCLUDED


class ShapeShifterDefinition
{
public:
	String contentName;
	std::function<ShapeShifterContent*(const String &)> createFunc;

	ShapeShifterDefinition(const String &_contentName, std::function<ShapeShifterContent*(const String &)> createFunc) :
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
	OwnedArray<ShapeShifterDefinition> defs;

	static ShapeShifterContent * createContent(const String &contentName)
	{
		for (auto &d : getInstance()->defs)
		{
			if (d->contentName == contentName) return d->createFunc(contentName);
		}

		return nullptr;
	}

};



#endif  // SHAPESHIFTERFACTORY_H_INCLUDED
