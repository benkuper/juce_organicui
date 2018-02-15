/*
  ==============================================================================

    FileParameter.h
    Created: 9 Mar 2016 12:29:30am
    Author:  bkupe

  ==============================================================================
*/

#ifndef FILEPARAMETER_H_INCLUDED
#define FILEPARAMETER_H_INCLUDED

class FileParameter : public StringParameter
{
public:
    FileParameter(const String &niceName, const String &description, const String &initialValue, bool enabled=true);
	virtual ~FileParameter();

    // need to override this function because var Strings comparison  is based on pointer (we need full string comp)
    virtual  void setValueInternal(var&) override;

	bool isRelativePath(const String &p);
	String getAbsolutePath() const;
	File getFile();

	static StringParameter * create() { return new FileParameter("New FileParameter", "",""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "File"; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileParameter)
};



#endif  // STRINGPARAMETER_H_INCLUDED
