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
class Automation;
class ParameterAutomation;

class ParameterListener
{
public:
	/** Destructor. */
	virtual ~ParameterListener() {}
	virtual void parameterValueChanged(Parameter*) {};
	virtual void parameterRangeChanged(Parameter*) {};
	virtual void parameterControlModeChanged(Parameter*) {}
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

	Parameter(const Type& type, const juce::String& niceName, const juce::String& description, juce::var initialValue, juce::var minValue, juce::var maxValue, bool enabled = true);
	virtual ~Parameter();

	juce::var defaultValue;
	juce::var value;
	juce::var lastValue;

	juce::SpinLock valueSetLock;

	//Range
	bool canHaveRange;
	bool rebuildUIOnRangeChange;
	juce::var minimumValue;
	juce::var maximumValue;

	//Control Mode
	bool lockManualControlMode;
	ControlMode controlMode;

	//Same value set behaviour
	bool alwaysNotify; //if true, will always notify of value change even if value is the same

	//Expression
	juce::String controlExpression;
	std::unique_ptr<ScriptExpression> expression;
	juce::WeakReference<Controllable> controlReference;

	//Reference
	std::unique_ptr<TargetParameter> referenceTarget;
	Parameter* referenceParameter;

	//Automation
	std::unique_ptr<ParameterAutomation> automation;
	bool canBeAutomated;

	//ColorStatus
	juce::HashMap<juce::var, juce::Colour> colorStatusMap;

	bool isComplex() const;
	virtual juce::StringArray getValuesNames();

	virtual void setRange(juce::var min, juce::var max);
	virtual void clearRange();
	virtual bool hasRange() const;

	bool isPresettable;
	bool isOverriden;
	bool forceSaveValue; //if true, will save value even if not overriden
	bool forceSaveRange; //will save range even if saveValueOnly is true

	virtual void setEnabled(bool value, bool silentSet = false, bool force = false) override;

	void setControlMode(ControlMode _mode);
	void setControlExpression(const juce::String&);
	void setReferenceParameter(Parameter* tp);
	virtual void setControlAutomation();

	virtual juce::var getValue(); //may be useful, or testing expression or references (for now, forward update from expression timer)
	virtual juce::var getLerpValueTo(juce::var targetValue, float weight);
	virtual void setWeightedValue(juce::Array<juce::var> values, juce::Array<float> weights) {} // to be overriden
	virtual juce::var getCroppedValue(juce::var originalValue);

	virtual void setDefaultValue(juce::var val, bool doResetValue = true);
	virtual void resetValue(bool silentSet = false);
	virtual juce::UndoableAction* setUndoableValue(juce::var oldValue, juce::var newValue, bool onlyReturnAction = false);
	virtual void setValue(juce::var _value, bool silentSet = false, bool force = false, bool forceOverride = true);
	virtual void setValueInternal(juce::var& _value);

	virtual bool checkValueIsTheSame(juce::var newValue, juce::var oldValue); //can be overriden to modify check behavior

	//For Number type parameters
	void setUndoableNormalizedValue(const juce::var& oldNormalizedValue, const juce::var& newNormalizedValue);
	void setNormalizedValue(const juce::var& normalizedValue, bool silentSet = false, bool force = false);
	juce::var getNormalizedValue() const;

	virtual bool setAttributeInternal(juce::String param, juce::var value) override;
	virtual juce::StringArray getValidAttributes() const override;

	//helpers for fast typing
	virtual float floatValue();
	virtual double doubleValue();
	virtual int intValue();
	virtual bool boolValue();
	virtual juce::String stringValue();

	void notifyValueChanged();

	//From Script Expression
	virtual void expressionValueChanged(ScriptExpression*) override;
	virtual void expressionStateChanged(ScriptExpression*) override;


	//Reference
	virtual void parameterValueChanged(Parameter* p) override;


	InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
	virtual ControllableDetectiveWatcher* getDetectiveWatcher() override;

	virtual DashboardItem* createDashboardItem() override;

	virtual bool shouldBeSaved() override;
	virtual juce::var getJSONDataInternal() override;
	virtual void loadJSONDataInternal(juce::var data) override;
	virtual void setupFromJSONData(juce::var data) override;

	virtual void getRemoteControlDataInternal(juce::var& data) override;
	virtual juce::var getRemoteControlValue();
	virtual juce::var getRemoteControlRange();
	
	static juce::var getValueFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var resetValueFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getRangeFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var setRangeFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var hasRangeFromScript(const juce::var::NativeFunctionArgs& a);

	juce::String getScriptTargetString() override;

	juce::ListenerList<ParameterListener, juce::Array<ParameterListener*,juce::CriticalSection>> listeners;
	void addParameterListener(ParameterListener* newListener) { listeners.add(newListener); }
	void removeParameterListener(ParameterListener* listener) { listeners.remove(listener); }

	// ASYNC
	class  ParameterEvent
	{
	public:
		enum Type { VALUE_CHANGED, BOUNDS_CHANGED, CONTROLMODE_CHANGED, EXPRESSION_STATE_CHANGED, UI_PARAMS_CHANGED };

		ParameterEvent(Type t, Parameter* p, juce::var v = juce::var()) :
			type(t), parameter(p), value(v)
		{
		}

		Type type;
		Parameter* parameter;
		juce::var value;
	};

	QueuedNotifier<ParameterEvent> queuedNotifier;
	typedef QueuedNotifier<ParameterEvent>::Listener AsyncListener;


	void addAsyncParameterListener(AsyncListener* newListener) { queuedNotifier.addListener(newListener); }
	void addAsyncCoalescedParameterListener(AsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncParameterListener(AsyncListener* listener) { queuedNotifier.removeListener(listener); }



	bool checkVarIsConsistentWithType();
	juce::WeakReference<Parameter>::Master masterReference;
	friend class juce::WeakReference<Parameter>;

	class ParameterAction :
		public ControllableAction
	{
	public:
		ParameterAction(Parameter* param) :
			ControllableAction(param),
			parameterRef(param)
		{
			controlAddress = param->getControlAddress();
		}

		juce::WeakReference<Parameter> parameterRef;
		juce::String controlAddress;

		Parameter* getParameter();
	};

	class ParameterSetValueAction :
		public ParameterAction
	{
	public:
		ParameterSetValueAction(Parameter* param, juce::var oldValue, juce::var newValue) :
			ParameterAction(param),
			oldValue(oldValue),
			newValue(newValue)
		{
			//DBG("New Parameter Set Value Action");
		}

		juce::var oldValue;
		juce::var newValue;

		bool perform() override;
		bool undo() override;
	};

	class ValueInterpolator :
		public InspectableListener
	{
	public:
		ValueInterpolator(juce::WeakReference<Parameter> p, juce::var targetValue, float time, Automation* a);
		~ValueInterpolator();

		juce::WeakReference<Parameter> parameter;
		juce::var valueAtStart;
		juce::var targetValue;
		float time;
		double timeAtStart;
		Automation* automation;
		void updateParams(juce::var targetValue, float time, Automation* a);
		bool update();

		void inspectableDestroyed(Inspectable*) override;

		class Manager :
			public juce::Thread
		{
		public:
			juce_DeclareSingleton(Manager, true);

			Manager();
			~Manager();

			juce::CriticalSection interpLock;

			void interpolate(juce::WeakReference<Parameter> p, juce::var targetValue, float time, Automation* a);

			juce::WeakReference<ValueInterpolator> getInterpolationWith(Parameter* p);
			void removeInterpolationWith(Parameter* p);

			juce::HashMap<Parameter*, juce::WeakReference<ValueInterpolator>, juce::DefaultHashFunctions, juce::CriticalSection> interpolatorMap;
			juce::OwnedArray<ValueInterpolator, juce::CriticalSection> interpolators;

			void run() override;

		};

		juce::WeakReference<ValueInterpolator>::Master masterReference;
		friend class juce::WeakReference<ValueInterpolator>;

	};

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameter)

};