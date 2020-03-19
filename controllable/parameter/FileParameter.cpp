#include "FileParameter.h"
/*
  ==============================================================================

    StringParameter.cpp
    Created: 9 Mar 2016 12:29:30am
    Author:  bkupe

  ==============================================================================
*/


FileParameter::FileParameter(const String & niceName, const String &description, const String & initialValue, bool enabled) :
    StringParameter(niceName, description, initialValue, enabled),
	customBasePath(""),
    forceRelativePath(false),
	forceAbsolutePath(false)

{
	defaultUI = FILE; 
	if(Engine::mainEngine != nullptr) Engine::mainEngine->addEngineListener(this);
}

FileParameter::~FileParameter()
{
	if(Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}

void FileParameter::setValueInternal(var &newVal)
{
	StringParameter::setValueInternal(newVal);

	if (newVal.toString().isNotEmpty())
	{
		if(File::isAbsolutePath(newVal.toString())) absolutePath = newVal.toString();
		else absolutePath = getBasePath().getChildFile(value.toString()).getFullPathName();

		File f = File::createFileWithoutCheckingPath(absolutePath);
		if (f.existsAsFile() && !forceAbsolutePath && (isRelativePath(newVal.toString()) || forceRelativePath))
		{
			value = File(absolutePath).getRelativePathFrom(getBasePath()).replace("\\", "/");
		}
	}
	
	value = value.toString().replace("\\", "/");
}

void FileParameter::setForceRelativePath(bool force)
{
	forceRelativePath = force;
	setValue(absolutePath, false, true);
}

bool FileParameter::isRelativePath(const String & p)
{
	if (p.isEmpty()) return false;
	if (Engine::mainEngine == nullptr) return false;
	if (File::isAbsolutePath(p)) return !File(p).getRelativePathFrom(getBasePath()).contains("..");
	return getBasePath().getChildFile(p).exists();
}

String FileParameter::getAbsolutePath() const
{
	if (value.toString().isEmpty()) return "";
	if (File::isAbsolutePath(value.toString())) return value.toString();
	if (Engine::mainEngine == nullptr) return value.toString();
	return getBasePath().getChildFile(value.toString()).getFullPathName();
}

File FileParameter::getBasePath() const
{
	return File(customBasePath).exists()? File(customBasePath):Engine::mainEngine->getFile().getParentDirectory();
}

File FileParameter::getFile()
{
	String p = getAbsolutePath();
	if (p.isEmpty()) return File();

	return File(p); 
}

var FileParameter::getJSONDataInternal()
{
	var data = StringParameter::getJSONDataInternal();
	if(forceRelativePath) data.getDynamicObject()->setProperty("relative", true);
	return data;
}

void FileParameter::loadJSONDataInternal(var data)
{
	StringParameter::loadJSONDataInternal(data);
	setForceRelativePath(data.getProperty("relative", false));
}

void FileParameter::fileSaved(bool savedAs)
{
	if(savedAs) setValue(absolutePath, false, true); //force re-evaluate relative path if changed
}
