/*
  ==============================================================================

    Automation.h
    Created: 11 Dec 2016 1:21:37pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class InspectableSelectionManager;
class AutomationTimelineUIBase;

class AutomationBase :
	public BaseManager<AutomationKeyBase>
{
public:
	AutomationBase(const String& name = "Automation");// , AutomationRecorder* recorder = nullptr, bool freeRange = false, bool allowKeysOutside = false, bool dedicatedSelectionManager = true);
	virtual ~AutomationBase();

	//Recorder
	AutomationRecorder * recorder;

	int numDimensions;

	//ui
	bool showUIInEditor;

	//Position and value
	bool allowKeysOutside; //allow keys positions to be outside automation timing
	FloatParameter * position;
	Parameter * value;
	FloatParameter * length;

	//snapping
	Array<float> snapPositions;
	BoolParameter * enableSnap;
	FloatParameter * snapSensitivity;

	bool hasRange();

	void setDedicatedSelectionManager(bool value);

	std::unique_ptr<InspectableSelectionManager> customSelectionManager;

	void setLength(float value, bool stretch = false, bool stickToEnd = false);
	
	virtual void updateCurrentValue() = 0;

	void removeKeysBetween(float start, float end);
	void removeAllSelectedKeys();

	void setSnapPositions(Array<float> positions);

	float getClosestSnapForPos(float pos, int start = -1, int end = -1);


	AutomationKeyBase * getClosestKeyForPos(float pos, int start = -1, int end = -1);
	AutomationKeyBase * getKeyAtPos(float pos);

	virtual Array<float> getValuesForPosition(float pos) = 0;


	virtual void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable *c) override;
	virtual void onContainerParameterChanged(Parameter *) override;

	static int compareTime(AutomationKeyBase * t1, AutomationKeyBase * t2);

	virtual AutomationTimelineUIBase* createTimelineUI() = 0;
	InspectableEditor * getEditor(bool isRoot) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationBase)
};

template<class T>
class Automation :
	public AutomationBase
{
public:
	Automation(const String& name = "Automation");// , AutomationRecorder* recorder = nullptr, bool freeRange = false, bool allowKeysOutside = false, bool dedicatedSelectionManager = true);
	virtual ~Automation() {}

	void clearRange();
	void setRange(T minValue, T maxValue);

	virtual void updateCurrentValue() override;
	T getValueForPosition(float pos);
	Array<float> getValuesForPosition(float pos) override;

	AutomationKey<T> * createItem() override;
	void addItems(HashMap<float, T> keys, bool removeExistingOverlappingKeys = true, bool addToUndo = true, bool autoSmoothCurve = false);
	AutomationKey<T> * addItem(const float position, const T value, bool addToUndo = true, bool reorder = false);
	Array<AutomationKeyBase*> addItemsFromClipboard(bool showWarning = false) override;

	virtual AutomationTimelineUIBase* createTimelineUI() override;
};



template<class T>
Automation<T>::Automation(const juce::String& name) : // , AutomationRecorder* recorder, bool freeRange, bool allowKeysOutside, bool dedicatedSelectionManager) :
	AutomationBase(name)
{
	if (std::is_same<T, float>())
	{
		value = addFloatParameter("Value", "Current value of the automation", 0);
		numDimensions = 1;
	}
	else if (std::is_same<T, Point<float>>())
	{
		value = addPoint2DParameter("Position", "Current value of the automation");
		numDimensions = 2;
	}
	else if (std::is_same<T, Vector3D<float>>())
	{
		value = addPoint3DParameter("Position", "Current value of the automation");
		numDimensions = 3;
	}

	value->hideInEditor = true;
	value->isControllableFeedbackOnly = true;

}

template<class T>
void Automation<T>::clearRange()
{
	value->clearRange();
	freeRange = true;
}

template<class T>
void Automation<T>::setRange(T minValue, T maxValue)
{
	value->setRange(minValue, maxValue);
	freeRange = !value->hasRange();
	for (auto& k : items)
	{
		if (freeRange) k->value->clearRange();
		else k->value->setRange(value->minimumValue, value->maximumValue);
	}
}

template<class T>
void Automation<T>::updateCurrentValue()
{
	value->setValue(getValueForPosition(position->floatValue()));
}

template <class T>
T Automation<T>::getValueForPosition(float pos)
{
	if (items.size() == 0) return 0;
	if (pos <= items[0]->position->floatValue()) return ((AutomationKey<T> *)items[0])->getValue();
	else if (pos >= items[items.size() - 1]->position->floatValue()) return ((AutomationKey<T>*)items[items.size() - 1])->getValue();

	AutomationKey<T> * k = (AutomationKey<T> *)getClosestKeyForPos(pos);
	if (k == nullptr) return 0;
	return k->getValue(((AutomationKey<T>*)items[items.indexOf(k) + 1]), pos);
}

template<class T>
Array<float> Automation<T>::getValuesForPosition(float pos)
{
	return Array<float>();
}

template<>
Array<float> Automation<float>::getValuesForPosition(float pos)
{
	float val = getValueForPosition(pos);

	Array<float> result;
	result.add(val);
	return  result;
}

template<>
Array<float> Automation<Point<float>>::getValuesForPosition(float pos)
{
	Point<float> val = getValueForPosition(pos);

	Array<float> result;
	result.add(val.x, val.y);
	return  result;
}

template<>
Array<float> Automation<Vector3D<float>>::getValuesForPosition(float pos)
{
	Vector3D<float> val = getValueForPosition(pos);

	Array<float> result;
	result.add(val.x, val.y, val.z);
	return result;
}


template <class T>
AutomationKey<T> * Automation<T>::createItem()
{
	AutomationKey<T> * k = new AutomationKey<T>();
	if (value->hasRange()) k->value->setRange(value->minimumValue, value->maximumValue);
	if(selectionManager != nullptr) k->setSelectionManager(selectionManager);
	if(!allowKeysOutside) k->position->setRange(0, length->floatValue());
	return k;
}

template <class T>
void Automation<T>::addItems(HashMap<float, T> keys, bool removeExistingOverlappingKeys, bool addToUndo, bool autoSmoothCurve)
{
	if(selectionManager != nullptr) selectionManager->setEnabled(false);

	//Array<UndoableAction *> actions;
	//if(removeExistingOverlappingKeys) actions.addArray(getRemoveKeysBetweenAction(keys[0].x, keys[keys.size() - 1].x));
	
	removeKeysBetween(keys[0].x, keys[keys.size() - 1].x);

	Array<AutomationKey<T> *> newKeys;

	int autoIndex = items.size();
	for (auto &k : keys)
	{
		AutomationKey<T> * ak = createItem();
		ak->setNiceName("Key " + String(autoIndex));
		ak->position->setValue(k.x);
		ak->value->setValue(k.y);
		if (autoSmoothCurve) ak->setEasing(Easing::BEZIER);
		newKeys.add(ak);

		autoIndex++;
	}

	BaseManager::addItems(newKeys);

	if(selectionManager != nullptr) selectionManager->setEnabled(true);
}

template <class T>
AutomationKey<T> * Automation<T>::addItem(const float _position, const T _value, bool addToUndo, bool reorder)
{
	AutomationKey<T> * k = createItem();
	k->position->setValue(_position);
	
	if (std::is_same<T, float>())	k->value->setValue(_value);
	else if (std::is_same<T, Point<float>>()) ((Point2DParameter *)(k->value))->setPoint(_value);
	else if (std::is_same<T, Vector3D<float>>())((Point3DParameter*)(k->value))->setVector(_value);

	BaseManager::addItem(k,var(), addToUndo);
	if (reorder) reorderItems();

	return k;

}

template <class T>
Array<AutomationKeyBase *> Automation<T>::addItemsFromClipboard(bool showWarning)
{
	Array<AutomationKeyBase *> keys = BaseManager::addItemsFromClipboard(showWarning);
	if (keys.isEmpty()) return nullptr;
	
	float minTime = keys[0]->position->floatValue();
	float maxTime = keys[0]->position->floatValue();
	for (auto &k : keys)
	{
		minTime = jmin(minTime, k->position->floatValue());
		maxTime = jmax(maxTime, k->position->floatValue());
	}

	float diffTime = position->floatValue() - minTime;
	
	//Remove all keys in paste range
	Array<AutomationKey<T> *> keysToRemove;
	for (auto &k : items)
	{
		if (keys.contains(k)) continue;
		if(k->position->floatValue() >= minTime+diffTime && k->position->floatValue() <= maxTime+diffTime) keysToRemove.add(k);
	}
	removeItems(keysToRemove);


	for (auto & k : keys) k->position->setValue(k->position->floatValue() + diffTime);
	reorderItems();

	return  keys;
}

template<class T>
AutomationTimelineUIBase* Automation<T>::createTimelineUI()
{
	return new AutomationTimelineUI<T>(this);
}
