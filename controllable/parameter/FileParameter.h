/*
  ==============================================================================

	FileParameter.h
	Created: 9 Mar 2016 12:29:30am
	Author:  bkupe

  ==============================================================================
*/
#pragma once




class FileParameter :
	public StringParameter,
	public EngineListener
{
public:
	FileParameter(const juce::String& niceName, const juce::String& description, const juce::String& initialValue, bool enabled = true);
	virtual ~FileParameter();

	juce::String customBasePath;

	juce::String absolutePath;
	juce::String fileTypeFilter;
	bool directoryMode;
	bool saveMode;
	
	bool autoReload;
	juce::Time lastModificationTime;

	bool forceAbsolutePath;
	bool forceRelativePath;

	// need to override this function because var Strings comparison  is based on pointer (we need full string comp)
	virtual void setValueInternal(juce::var&) override;

	void setForceRelativePath(bool value);
	void setForceAbsolutePath(bool value);


	bool isRelativePath(const juce::String& p);
	juce::String getAbsolutePath() const;
	juce::File getBasePath() const;
	juce::File getFile();


	void setAutoReload(bool value);
	void checkForHotReload();

	juce::var getJSONDataInternal() override;
	void loadJSONDataInternal(juce::var data) override;

	void fileSaved(bool savedAs) override;

	static juce::var readFileFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var writeFileFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var writeBytesFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getAbsolutePathFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var launchFileFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var listFilesFromScript(const juce::var::NativeFunctionArgs& a);

	virtual bool setAttributeInternal(juce::String param, juce::var paramVal) override;
	virtual juce::StringArray getValidAttributes() const override;

	static StringParameter* create() { return new FileParameter("New FileParameter", "", ""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "File"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileParameter)
};

class FileCheckTimer :
	public juce::Timer
{
public:
	juce_DeclareSingleton(FileCheckTimer, true);
	FileCheckTimer() { startTimer(500); }
	~FileCheckTimer() {}

	juce::Array<FileParameter*, juce::CriticalSection> params;
	void registerParam(FileParameter* fp);
	void unregisterParam(FileParameter* fp);

	void timerCallback() override;
};