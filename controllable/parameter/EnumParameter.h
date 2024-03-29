/*
  ==============================================================================

    EnumParameter.h
    Created: 29 Sep 2016 5:34:59pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class EnumParameterUI;
class EnumParameterButtonBarUI;


class EnumParameter : public Parameter
{
public:

	EnumParameter(const juce::String &niceName, const juce::String &description, bool enabled = true);
	~EnumParameter();

	struct EnumValue
	{
		EnumValue(juce::String key, juce::var value) : key(key), value(value) {}
		juce::String key;
		juce::var value;
	};

	EnumParameter * addOption(juce::String key, juce::var data, bool selectIfFirstOption = true); //daisy chain
	void updateOption(int index, juce::String key, juce::var data, bool addIfNotThere = false);
	void removeOption(juce::String key);
	void setOptions(juce::Array<EnumValue *> options);
	void clearOptions();

	void updateArgDescription();

	

	juce::OwnedArray<EnumValue> enumValues;

	juce::var getValue() override;
	juce::var getValueData();

	template<class T>
	T getValueDataAsEnum() {
		EnumValue * ev = getEntryForKey(value.toString());
		if (ev == nullptr) return (T)0;
		return (T)(int)ev->value; 
	}

	juce::String getValueKey();

	int getIndexForKey(juce::StringRef key);
	EnumValue * getEntryForKey(juce::StringRef key);

	juce::StringArray getAllKeys();

	void setValueWithData(juce::var data);
	void setValueWithKey(juce::String data);
	void setPrev(bool loop = true, bool addToUndo = false);
	void setNext(bool loop = true, bool addToUndo = false);

	bool checkValueIsTheSame(juce::var oldValue, juce::var newValue) override;

	juce::var getJSONDataInternal() override;
	void loadJSONDataInternal(juce::var data) override;
	virtual void setupFromJSONData(juce::var data) override;

	juce::var getRemoteControlValue() override;
	juce::var getRemoteControlRange() override;

	static juce::var getValueKeyFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var addOptionFromScript(const juce::var::NativeFunctionArgs &a);
	static juce::var removeOptionsFromScript(const juce::var::NativeFunctionArgs &a);
	static juce::var setValueWithDataFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getAllOptionsFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getOptionAtFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getIndexFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var setNextFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var setPreviousFromScript(const juce::var::NativeFunctionArgs& a);


	EnumParameterUI* createUI(juce::Array<EnumParameter*> parameters = {});
	EnumParameterButtonBarUI* createButtonBarUI(juce::Array<EnumParameter*> parameters = {});
	ControllableUI * createDefaultUI(juce::Array<Controllable*> controllables = {}) override;

	virtual DashboardItem* createDashboardItem() override;


	//Listener
	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void enumOptionAdded(EnumParameter*, const juce::String&) = 0;
		virtual void enumOptionUpdated(EnumParameter *, int index, const juce::String & prevKey, const juce::String &newKey) = 0;
		virtual void enumOptionRemoved(EnumParameter *, const juce::String &) = 0;
	};

	juce::ListenerList<Listener> enumListeners;
	void addEnumParameterListener(Listener* newListener) { enumListeners.add(newListener); }
	void removeEnumParameterListener(Listener* listener) { enumListeners.remove(listener); }

	DECLARE_ASYNC_EVENT(EnumParameter, EnumParameter, enumParameter, ENUM_LIST(ENUM_OPTION_ADDED, ENUM_OPTION_UPDATED, ENUM_OPTION_REMOVED))

	static EnumParameter * create() { return new EnumParameter("new Enum Parameter",""); }
	virtual juce::String getTypeString() const override { return getTypeStringStatic(); }
	static juce::String getTypeStringStatic() { return "Enum"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnumParameter)
};