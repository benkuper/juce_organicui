/*
  ==============================================================================

	FileParameter.cpp
	Created: 9 Mar 2016 12:29:30am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

FileParameter::FileParameter(const String& niceName, const String& description, const String& initialValue, bool enabled) :
	StringParameter(niceName, description, initialValue, enabled),
	customBasePath(""),
	directoryMode(false),
	saveMode(false),
	forceAbsolutePath(false),
	forceRelativePath(false)
{
	defaultUI = FILE;

	scriptObject.setMethod("readFile", FileParameter::readFileFromScript);
	scriptObject.setMethod("writeFile", FileParameter::writeFileFromScript);
	scriptObject.setMethod("writeBytes", FileParameter::writeBytesFromScript);
	scriptObject.setMethod("getAbsolutePath", FileParameter::getAbsolutePathFromScript);
	scriptObject.setMethod("launchFile", FileParameter::launchFileFromScript);
	scriptObject.setMethod("listFiles", FileParameter::listFilesFromScript);

	if (Engine::mainEngine != nullptr) Engine::mainEngine->addEngineListener(this);
}

FileParameter::~FileParameter()
{
	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}

void FileParameter::setValueInternal(var& newVal)
{
	StringParameter::setValueInternal(newVal);

	if (newVal.toString().isNotEmpty())
	{
		if (File::isAbsolutePath(newVal.toString())) absolutePath = newVal.toString();
		else absolutePath = getBasePath().getChildFile(value.toString()).getFullPathName();

		File f = File::createFileWithoutCheckingPath(absolutePath);
		if (f.exists() && !forceAbsolutePath && (isRelativePath(newVal.toString()) || forceRelativePath))
		{
			value = File(absolutePath).getRelativePathFrom(getBasePath()).replace("\\", "/");
		}
	}

	value = value.toString().replace("\\", "/");
}

void FileParameter::setForceRelativePath(bool force)
{
	forceRelativePath = force;
	forceAbsolutePath = !force;
	setValue(absolutePath, false, true);
}

void FileParameter::setForceAbsolutePath(bool force)
{
	forceRelativePath = !force;
	forceAbsolutePath = force;
	setValue(absolutePath, false, true);
}

bool FileParameter::isRelativePath(const String& p)
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
	if (File(customBasePath).exists()) return File(customBasePath);
	if (Engine::mainEngine->getFile().exists()) return Engine::mainEngine->getFile().getParentDirectory();
	return File::getCurrentWorkingDirectory();
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
	if (forceRelativePath) data.getDynamicObject()->setProperty("relative", true);

	if (!saveValueOnly)
	{
		if (directoryMode) data.getDynamicObject()->setProperty("directoryMode", true);
	}

	return data;
}

void FileParameter::loadJSONDataInternal(var data)
{
	if (!saveValueOnly) directoryMode = data.getProperty("directoryMode", directoryMode);

	setForceRelativePath(data.getProperty("relative", false));
	StringParameter::loadJSONDataInternal(data);
}

void FileParameter::fileSaved(bool savedAs)
{
	if (savedAs && !forceAbsolutePath) setValue(absolutePath, false, true); //force re-evaluate relative path if changed
}

var FileParameter::readFileFromScript(const juce::var::NativeFunctionArgs& a)
{
	FileParameter* p = getObjectFromJS<FileParameter>(a);
	File f = p->getFile();
	if (!f.existsAsFile()) return var();

	if (a.numArguments >= 1 && (int)a.arguments[0])
	{
		return JSON::parse(f);
	}
	else
	{
		FileInputStream fs(f);
		return fs.readEntireStreamAsString();
	}
}

var FileParameter::writeFileFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return false;

	FileParameter* p = getObjectFromJS<FileParameter>(a);
	File f = p->getFile();

	bool overwriteIfExists = a.numArguments > 1 ? ((int)a.arguments[1] > 0) : false;
	if (f.existsAsFile())
	{
		if (overwriteIfExists) f.deleteFile();
		else
		{
			LOG("File already exists : " << f.getFileName() << ", you need to enable overwrite to replace its content.");
			return false;
		}
	}

	FileOutputStream fs(f);
	if (a.arguments[0].isObject())
	{
		JSON::writeToStream(fs, a.arguments[0]);
		return true;
	}

	return fs.writeText(a.arguments[0].toString(), false, false, "\n");
}

var FileParameter::writeBytesFromScript(const juce::var::NativeFunctionArgs& a)
{
	if (a.numArguments == 0) return false;

	FileParameter* p = getObjectFromJS<FileParameter>(a);
	File f = p->getFile();

	bool overwriteIfExists = a.numArguments > 1 ? ((int)a.arguments[1] > 0) : false;
	if (f.existsAsFile())
	{
		if (overwriteIfExists) f.deleteFile();
		else
		{
			LOG("File already exists : " << f.getFileName() << ", you need to enable overwrite to replace its content.");
			return false;
		}
	}

	FileOutputStream fs(f);
	Array<uint8_t> bytes;
	if (a.arguments[0].isArray())
	{
		for (int i = 0; i < a.arguments[0].size(); i++)
		{
			bytes.add((uint8_t)(int)a.arguments[0][i]);
		}
	}
	else
	{
		LOGWARNING("Write bytes needs an array of bytes");
		return false;
	}

	return fs.write(bytes.getRawDataPointer(), bytes.size());
}

var FileParameter::getAbsolutePathFromScript(const juce::var::NativeFunctionArgs& a)
{
	return getObjectFromJS<FileParameter>(a)->getAbsolutePath();

}

var FileParameter::launchFileFromScript(const juce::var::NativeFunctionArgs& a)
{
	FileParameter* p = getObjectFromJS<FileParameter>(a);
	File f = p->getFile();

	if (f.existsAsFile()) f.startAsProcess(a.numArguments > 0 ? a.arguments[0].toString() : "");
	return var();
}

var FileParameter::listFilesFromScript(const juce::var::NativeFunctionArgs& a)
{
	FileParameter* p = getObjectFromJS<FileParameter>(a);
	File f = p->getFile();

	var result;

	if (f.exists() && f.isDirectory())
	{
		int whatToLook = 0;
		if (a.numArguments == 0 || a.arguments[0]) whatToLook += File::findFiles;
		if (a.numArguments > 1 && a.arguments[1]) whatToLook += File::findDirectories;
		Array<File> files = f.findChildFiles(whatToLook, a.numArguments > 2 && a.arguments[2], a.numArguments > 3 ? a.arguments[3].toString() : "*");

		for (auto& ff : files) result.append(ff.getFullPathName());
	}

	return result;
}


bool FileParameter::setAttributeInternal(String param, var paramVal)
{
	if (param == "directoryMode") directoryMode = paramVal;
	else if (param == "saveMode") saveMode = paramVal;
	else
	{
		return StringParameter::setAttributeInternal(param, paramVal);
	}

	return true;
}

StringArray FileParameter::getValidAttributes() const
{
	StringArray att = StringParameter::getValidAttributes();
	att.add("directoryMode");
	att.add("saveMode");
	return att;
}
