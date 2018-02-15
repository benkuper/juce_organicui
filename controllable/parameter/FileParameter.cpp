#include "FileParameter.h"
/*
  ==============================================================================

    StringParameter.cpp
    Created: 9 Mar 2016 12:29:30am
    Author:  bkupe

  ==============================================================================
*/


FileParameter::FileParameter(const String & niceName, const String &description, const String & initialValue, bool enabled) :
    StringParameter(niceName, description, initialValue, enabled)
{
	defaultUI = FILE;
}

FileParameter::~FileParameter()
{
}

void FileParameter::setValueInternal(var &newVal)
{
	StringParameter::setValueInternal(newVal);
	if (isRelativePath(value.toString()))
	{
		value = File(newVal.toString()).getRelativePathFrom(Engine::mainEngine->getFile().getParentDirectory());
	}
}

bool FileParameter::isRelativePath(const String & p)
{
	if (File::isAbsolutePath(p)) return !File(p).getRelativePathFrom(Engine::mainEngine->getFile().getParentDirectory()).contains("..");
	return Engine::mainEngine->getFile().getParentDirectory().getChildFile(p).exists();
}

String FileParameter::getAbsolutePath() const
{
	if (File::isAbsolutePath(value.toString())) return value.toString();
	return Engine::mainEngine->getFile().getParentDirectory().getChildFile(value.toString()).getFullPathName();
}

File FileParameter::getFile()
{
	return File(getAbsolutePath());
}