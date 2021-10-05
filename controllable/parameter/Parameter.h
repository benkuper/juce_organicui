/*
 ==============================================================================

 Parameter.h
 Created: 8 Mar 2016 1:08:19pm
 Author:  bkupe

 ==============================================================================
 */

#pragma once


 //Listener
class Parameter;
class TargetParameter;
class ParameterAutomation;

class ParameterListener
{
public:
	/** Destructor. */
	virtual ~ParameterListener() {}
	virtual void parameterValueChanged(Parameter *) {};
	virtual void parameterRangeChanged(Parameter *) {};
	virtual void parameterControlModeChanged(Parameter *) {}
};

class Parameter : 
	public Controllable,
	public ExpressionListener,
	public ParameterListener
{
public:
	enum ControlMode {
		MANUAL,
		EXPRESSION,
		REFERENCE,
		AUTOMATION
	};

    Parameter(const Type &type, const String & niceName, const String &description, var initialValue, var minValue, var maxValue, bool enabled = true);
	virtual ~Parameter();

    var defaultValue;
    var value;
    var lastValue;

	SpinLock valueSetLock;

	//Range
	bool canHaveRange;
	bool rebuildUIOnRangeChange;
    var minimumValue;
    var maximumValue;

	//Control Mode
	bool lockManualControlMode;
	ControlMode controlMode;

	//Same value set behaviour
	bool alwaysNotify; //if true, will always notify of value change even if value is the same

	//Expression
	String controlExpression;
	std::unique_ptr<ScriptExpression> expression;
	WeakReference<Controllable> controlReference;

	//Reference
	std::unique_ptr<TargetParameter> referenceTarget;
	Parameter * referenceParameter;

	//Automation
	std::unique_ptr<ParameterAutomation> automation;
	bool canBeAutomated;

	//ColorStatus
	HashMap<var, Colour> colorStatusMap;

	bool isComplex();
	virtual StringArray getValuesNames();

	virtual void setRange(var min, var max);
	virtual void clearRange();
	virtual bool hasRange();

	bool isPresettable;
    bool isOverriden;
	bool forceSaveValue; //if true, will save value even if not overriden
	bool forceSaveRange; //will save range even if saveValueOnly is true

	virtual void setEnabled(bool value, bool silentSet = false, bool force = false) override;

	void setControlMode(ControlMode _mode);
	void setControlExpression(const String &);
	void setReferenceParameter(Parameter * tp);
	virtual void setControlAutomation();

	virtual var getValue(); //may be useful, or testing expression or references (for now, forward update from expression timer)
	virtual var getLerpValueTo(var targetValue, float weight);
	virtual void setWeightedValue(Array<var> values, Array<float> weights) {} // to be overriden
	virtual var getCroppedValue(var originalValue);

    virtual void resetValue(bool silentSet = false);
	virtual UndoableAction * setUndoableValue(var oldValue, var newValue, bool onlyReturnAction = false);
    virtual void setValue(var _value, bool silentSet = false, bool force = false, bool forceOverride = true);
    virtual void setValueInternal(var & _value);

	virtual bool checkValueIsTheSame(var newValue, var oldValue); //can be overriden to modify check behavior

    //For Number type parameters
	void setUndoableNormalizedValue(const var &oldNormalizedValue, const var &newNormalizedValue);
	void setNormalizedValue(const var &normalizedValue, bool silentSet = false, bool force = false);
	var getNormalizedValue();
	
	virtual void setAttribute(String param, var value) override;
	virtual StringArray getValidAttributes() const override;

    //helpers for fast typing
	virtual float floatValue();
	virtual double doubleValue();
	virtual int intValue();
	virtual bool boolValue();
	virtual String stringValue();

    void notifyValueChanged();

	//From Script Expression
	virtual void expressionValueChanged(ScriptExpression *) override;
	virtual void expressionStateChanged(ScriptExpression *) override;
	

	//Reference
	virtual void parameterValueChanged(Parameter * p) override;


	InspectableEditor* getEditorInternal(bool isRoot) override;
	virtual ControllableDetectiveWatcher* getDetectiveWatcher() override;

	virtual DashboardItem* createDashboardItem() override;

	virtual bool shouldBeSaved() override;
	virtual var getJSONDataInternal() override;
	virtual void loadJSONDataInternal(var data) override;
	virtual void setupFromJSONData(var data) override;
	
	static var getValueFromScript(const juce::var::NativeFunctionArgs& a);
	static var getRangeFromScript(const juce::var::NativeFunctionArgs& a);
	static var setRangeFromScript(const juce::var::NativeFunctionArgs& a);
	static var hasRangeFromScript(const juce::var::NativeFunctionArgs &a);

	String getScriptTargetString() override;

    ListenerList<ParameterListener> listeners;
    void addParameterListener(ParameterListener* newListener) { listeners.add(newListener); }
    void removeParameterListener(ParameterListener* listener) { listeners.remove(listener); }

    // ASYNC
    class  ParameterEvent
	{
    public:
		enum Type { VALUE_CHANGED, BOUNDS_CHANGED, CONTROLMODE_CHANGED, EXPRESSION_STATE_CHANGED };

		ParameterEvent(Type t,Parameter * p,var v = var()) :
			type(t),parameter(p),value(v)
		{
		}

		Type type;
        Parameter * parameter;
        var value;
    };

    QueuedNotifier<ParameterEvent> queuedNotifier;
    typedef QueuedNotifier<ParameterEvent>::Listener AsyncListener;


    void addAsyncParameterListener(AsyncListener* newListener) { queuedNotifier.addListener(newListener); }
    void addAsyncCoalescedParameterListener(AsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
    void removeAsyncParameterListener(AsyncListener* listener) { queuedNotifier.removeListener(listener); }
	 


	bool checkVarIsConsistentWithType();
	WeakReference<Parameter>::Master masterReference;
	friend class WeakReference<Parameter>;

	class ParameterAction :
		public ControllableAction
	{
	public:
		ParameterAction(Parameter * param) :
			ControllableAction(param),
			parameterRef(param)
		{
			controlAddress = param->getControlAddress();
		}

		WeakReference<Parameter> parameterRef;
		String controlAddress;

		Parameter * getParameter();
	};

	class ParameterSetValueAction :
		public ParameterAction
	{
	public:
		ParameterSetValueAction(Parameter * param, var oldValue, var newValue) :
			ParameterAction(param),
			oldValue(oldValue), 
			newValue(newValue)
		{
			//DBG("New Parameter Set Value Action");
		}

		var oldValue;
		var newValue;
		
		bool perform() override;
		bool undo() override;
	};

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameter)

};