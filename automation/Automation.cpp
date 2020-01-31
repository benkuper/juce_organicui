#include "Automation.h"
/*  ==============================================================================

	Automation.cpp
	Created: 11 Dec 2016 1:21:37pm
	Author:  Ben

  ==============================================================================
*/

Automation::Automation(const String &name, int numDimensions, AutomationRecorder * _recorder, bool allowKeysOutside, bool dedicatedSelectionManager) :
	BaseManager(name),
	numDimensions(numDimensions),
	recorder(_recorder),
	hasRange(false),
	showUIInEditor(false),
	allowKeysOutside(allowKeysOutside)
{

	notifyStructureChangeWhenLoadingData = false;
		
	comparator.compareFunc = &Automation::compareTime;

	itemDataType = "AutomationKey"; 
	editorCanBeCollapsed = false;

	length = addFloatParameter("Length", "Length of the automation", 1, .1f, INT32_MAX);
	length->defaultUI = FloatParameter::TIME;
	length->hideInEditor = true;

	position = addFloatParameter("Position", "The current position in the automation. Used for automatic retrieve value and feedback.", 0, 0, length->floatValue());
	position->hideInEditor = true;

	for (int i = 0; i < numDimensions; i++)
	{
		FloatParameter * value = addFloatParameter("Value", "The current value, depending on the position", 0);
		value->hideInEditor = true;
		value->isControllableFeedbackOnly = true;
		values.add(value);

		minimumValues.add(INT32_MIN);
		maximumValues.add(INT32_MAX);
	}

	enableSnap = addBoolParameter("Enable Snap", "If enabled, moving keys will be automatically adjusted to interesting positions such as automation position", false);
	snapSensitivity = addFloatParameter("Snap Sensitivity", "Controls the sensitivity of the snapping, if enabled.\nThe greater the value, the more likely a position will be snapped.", .5f, 0, 3, false);
	enableSnap->hideInEditor = true;
	snapSensitivity->hideInEditor = true; 

	//selectItemWhenCreated = false;
	if (dedicatedSelectionManager)
	{
		customSelectionManager.reset(new InspectableSelectionManager(false));
		showInspectorOnSelect = false;
		selectionManager = customSelectionManager.get();
	}

	helpID = "Automation";
}

Automation::~Automation()
{
}

void Automation::removeKeysBetween(float start, float end)
{

	Array<AutomationKey *> keysToRemove;
	for (auto &k : items) if (k->position->floatValue() >= start && k->position->floatValue() <= end) keysToRemove.add(k);

	removeItems(keysToRemove);
}

void Automation::removeAllSelectedKeys()
{
	Array<AutomationKey *> keysToRemove;
	for (auto &k : items) if (k->isSelected) keysToRemove.add(k);
	for (auto &k : keysToRemove) removeItem(k);
}

void Automation::setSnapPositions(Array<float> positions)
{
	snapPositions = positions;
}

float Automation::getClosestSnapForPos(float pos, int start, int end)
{
	if (snapPositions.size() == 0) return pos;

	if (start == -1) start = 0;
	if (end == -1) end = snapPositions.size() - 1;

	if (pos < snapPositions[0]) return snapPositions[0];
	if (pos > snapPositions[snapPositions.size() - 1]) return snapPositions[snapPositions.size() - 1];

	if (end - start <= 1) return snapPositions[start];

	int midIndex = (int)floor((start + end) / 2);
	float medPos = snapPositions[midIndex];

	if (pos == medPos) return snapPositions[midIndex];

	else if (pos > medPos)
	{
		return getClosestSnapForPos(pos, midIndex, end);
	} else
	{
		return getClosestSnapForPos(pos, start, midIndex);
	}
}

void Automation::clearRange()
{
	for(auto &value: values) value->clearRange();
	hasRange = false;
}


void Automation::setRange(Array<float> minValues, Array<float> maxValues)
{
	hasRange = true;
	
	for (int i = 0; i < numDimensions;i++) values[i]->setRange(minValues[i], maxValues[i]);

	for (auto &k : items)
	{
		if (hasRange)k->setRange(minValues, maxValues);
		else  k->clearRange();
	}
}

AutomationKey * Automation::getClosestKeyForPos(float pos, int start, int end)
{
	if (items.size() == 0) return nullptr;

	if (start == -1) start = 0;
	if (end == -1) end = items.size() - 1;

	if (pos <= items[0]->position->floatValue()) return items[0];
	if (pos >= items[items.size() - 1]->position->floatValue()) return items[items.size() - 1];

	if (end - start <= 1) return items[start];

	int midIndex = (int)floor((start + end) / 2);
	float medPos = items[midIndex]->position->floatValue();

	if (pos == medPos) return items[midIndex];

	else if (pos > medPos)
	{
		return getClosestKeyForPos(pos, midIndex, end);
	} else
	{
		return getClosestKeyForPos(pos, start, midIndex);
	}
}

AutomationKey * Automation::getKeyAtPos(float pos)
{
	AutomationKey * k = getClosestKeyForPos(pos);
	if (k == nullptr) return nullptr;
	if(k->position->floatValue() == pos) return k;
	return nullptr;
}

void Automation::setLength(float newLength, bool stretch, bool stickToEnd)
{
	if (length->floatValue() == newLength) return;


	if (stretch)
	{
		float stretchFactor = newLength / length->floatValue();
		if (stretchFactor > 1) length->setValue(newLength); //if stretching, we have first to expand the length in case keys are not allowed outside
		for (auto &k : items) k->position->setValue(k->position->floatValue()*stretchFactor);
		if (stretchFactor < 1) length->setValue(newLength); //if reducing, we have to first reduce keys and then we can reduce the length, in case keys are not allowed outside
	}
	else
	{
		float lengthDiff = newLength - length->floatValue();
		
		length->setValue(newLength); // just change the value, nothing unusual

		if (stickToEnd) for (auto &k : items) k->position->setValue(k->position->floatValue() + lengthDiff);
	}

	
}

Array<float> Automation::getValuesForPosition(float pos)
{
	if (items.size() == 0) return 0;
	if (pos <= items[0]->position->floatValue()) return items[0]->getValues();
	else if (pos >= items[items.size() - 1]->position->floatValue()) return items[items.size() - 1]->getValues();

	AutomationKey * k = getClosestKeyForPos(pos);
	if (k == nullptr) return 0;
	return k->getValues(items[items.indexOf(k) + 1], pos);
}

Array<float>  Automation::getNormalizedValuesForPosition(float pos)
{
	Array<float> curValues = getValuesForPosition(pos);
	Array<float> result;
	for (int i = 0; i < numDimensions; i++) result.add(jmap<float>(curValues[i], values[i]->minimumValue, values[i]->maximumValue, 0, 1));
	return result;
}

AutomationKey * Automation::createItem()
{
	AutomationKey * k = new AutomationKey(numDimensions,minimumValues, maximumValues);
	if(selectionManager != nullptr) k->setSelectionManager(selectionManager);
	if(!allowKeysOutside) k->position->setRange(0, length->floatValue());
	return k;
}

void Automation::addItems(Array<float> positions, Array<Array<float>> keyValues, bool removeExistingOverlappingKeys, bool addToUndo, bool autoSmoothCurve)
{
	if(selectionManager != nullptr) selectionManager->setEnabled(false);

	if (positions.size() != keyValues.size())
	{
		jassertfalse;
		return;
	}

	//Array<UndoableAction *> actions;
	//if(removeExistingOverlappingKeys) actions.addArray(getRemoveKeysBetweenAction(keys[0].x, keys[keys.size() - 1].x));
	
	DBG("Add items in Automation");
	removeKeysBetween(positions[0], positions[positions.size() - 1]);

	Array<AutomationKey *> newKeys;

	int autoIndex = items.size();
	int numNewKeys = positions.size();
	for (int i = 0; i < numNewKeys; i++)
	{
		AutomationKey * ak = createItem();
		ak->setNiceName("Key " + String(autoIndex));
		ak->position->setValue(positions[i]);
		ak->setValues(keyValues[i]);
		if (autoSmoothCurve) ak->setEasing(Easing::BEZIER);
		newKeys.add(ak);

		autoIndex++;
	}

	BaseManager::addItems(newKeys);

	if(selectionManager != nullptr) selectionManager->setEnabled(true);


}

AutomationKey * Automation::addItem(const float _position, const Array<float> _values, bool addToUndo, bool reorder)
{
	AutomationKey * k = createItem();
	k->position->setValue(_position);
	k->setValues(_values);
	BaseManager::addItem(k,var(), addToUndo);
	if (reorder) reorderItems();

	return k;

}

Array<AutomationKey*> Automation::addItemsFromClipboard(bool showWarning)
{
	Array<AutomationKey *> keys = BaseManager::addItemsFromClipboard(showWarning);
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
	Array<AutomationKey *> keysToRemove;
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

void Automation::onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
	AutomationKey * t = dynamic_cast<AutomationKey *>(cc);

	if (t != nullptr)
	{
		if (enableSnap->boolValue() && c == t->position)
		{
			float curPos = position->floatValue();
			float tPos = t->position->floatValue();
			if (fabsf(curPos - tPos) < snapSensitivity->floatValue())
			{
				t->position->setValue(curPos);
				return;
			} else
			{
				float newPos = getClosestSnapForPos(tPos);
				if (newPos != tPos && fabsf(newPos - tPos) < snapSensitivity->floatValue())
				{
					t->position->setValue(newPos);
					return;
				}
			}
		}

		Array<float> curValues = getValuesForPosition(position->floatValue());
		for (int i = 0; i < numDimensions; i++) values[i]->setValue(curValues[i]);

		if (c == t->position)
		{
			int index = items.indexOf(t);
			bool swapped = true;

			if (index > 0 && t->position->floatValue() < items[index - 1]->position->floatValue()) items.swap(index, index - 1);
			else if (index < items.size() - 1 && t->position->floatValue() > items[index + 1]->position->floatValue()) items.swap(index, index + 1);
			else swapped = false;

			if (swapped)
			{
				baseManagerListeners.call(&ManagerListener::itemsReordered);
				managerNotifier.addMessage(new BaseManager::ManagerEvent(BaseManager::ManagerEvent::ITEMS_REORDERED));
			}
		}
	}
}

void Automation::onContainerParameterChanged(Parameter * p)
{
	if (p == position)
	{
		Array<float> curValues = getValuesForPosition(position->floatValue());
		for (int i = 0; i < numDimensions; i++) values[i]->setValue(curValues[i]);
	} else if (p == enableSnap)
	{
		snapSensitivity->setEnabled(enableSnap->boolValue());
	} else if (p == length)
	{
		position->setRange(0, length->floatValue());
		if(!allowKeysOutside) for (auto &k : items) k->position->setRange(0, length->floatValue());
	}
}

int Automation::compareTime(AutomationKey * t1, AutomationKey * t2)
{
	if (t1->position->floatValue() < t2->position->floatValue()) return -1;
	else if (t1->position->floatValue() > t2->position->floatValue()) return 1;
	return 0;
}

InspectableEditor * Automation::getEditor(bool isRoot)
{
	return new AutomationEditor(this, isRoot);
}
