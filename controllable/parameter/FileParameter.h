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
    FileParameter(const String &niceName, const String &description, const String &initialValue, bool enabled=true);
	virtual ~FileParameter();

	String customBasePath;

	String absolutePath;
	String fileTypeFilter;
    bool directoryMode;

	bool forceAbsolutePath;
    bool forceRelativePath;
    
    // need to override this function because var Strings comparison  is based on pointer (we need full string comp)
    virtual void setValueInternal(var&) override;
	
	void setForceRelativePath(bool value);

	bool isRelativePath(const String &p);
	String getAbsolutePath() const;
	File getBasePath() const;
	File getFile();

	var getJSONDataInternal() override;
	void loadJSONDataInternal(var data) override;

	void fileSaved(bool savedAs) override;

	static var readFileFromScript(const juce::var::NativeFunctionArgs& a);
	static var writeFileFromScript(const juce::var::NativeFunctionArgs& a);
	static var getAbsolutePathFromScript(const juce::var::NativeFunctionArgs& a);
	static var launchFileFromScript(const juce::var::NativeFunctionArgs& a);


	static StringParameter * create() { return new FileParameter("New FileParameter", "",""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "File"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileParameter)
};
