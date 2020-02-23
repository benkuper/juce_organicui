/*
  ==============================================================================

    FileParameter.cpp
    Created: 9 Mar 2016 12:29:30am
    Author:  bkupe

  ==============================================================================
*/


FileParameter::FileParameter(const String & niceName, const String &description, const String & initialValue, bool enabled) :
    StringParameter(niceName, description, initialValue, enabled),
	customBasePath(""),
	forceAbsolutePath(false),
	directoryMode(false),
	forceRelativePath(false)
{
	defaultUI = FILE; 

	scriptObject.setMethod("readFile", FileParameter::readFileFromScript);
	scriptObject.setMethod("writeFile", FileParameter::writeFileFromScript);
	scriptObject.setMethod("getAbsolutePath", FileParameter::getAbsolutePathFromScript);

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

var FileParameter::getAbsolutePathFromScript(const juce::var::NativeFunctionArgs& a)
{
	return getObjectFromJS<FileParameter>(a)->getAbsolutePath();

}