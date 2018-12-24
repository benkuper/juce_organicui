#include "Automation.h"
/*  ==============================================================================

	Automation.cpp
	Created: 11 Dec 2016 1:21:37pm
	Author:  Ben

  ==============================================================================
*/

AutomationKeyComparator Automation::comparator;

Automation::Automation(const String &name, AutomationRecorder * _recorder, bool freeRange, bool dedicatedSelectionManager) :
	BaseManager(name),
	recorder(_recorder),
	showUIInEditor(false),
	freeRange(freeRange)
{
	itemDataType = "AutomationKey"; 
	editorCanBeCollapsed = false;

	length = addFloatParameter("Length", "Length of the automation", 1, .1f, INT32_MAX);
	length->defaultUI = FloatParameter::TIME;
	length->hideInEditor = true;

	position = addFloatParameter("Position", "The current position in the automation. Used for automatic retrieve value and feedback.", 0, 0, length->floatValue());
	position->hideInEditor = true;

	value = addFloatParameter("Value", "The current value, depending on the position", 0, freeRange?INT32_MIN:0,freeRange?INT32_MAX:1);
	value->hideInEditor = true;
	value->isControllableFeedbackOnly = true;

	enableSnap = addBoolParameter("Enable Snap", "If enabled, moving keys will be automatically adjusted to interesting positions such as automation position", false);
	snapSensitivity = addFloatParameter("Snap Sensitivity", "Controls the sensitivity of the snapping, if enabled.\nThe greater the value, the more likely a position will be snapped.", .5f, 0, 3, false);
	enableSnap->hideInEditor = true;
	snapSensitivity->hideInEditor = true; 

	//selectItemWhenCreated = false;
	if (dedicatedSelectionManager)
	{
		customSelectionManager = new InspectableSelectionManager(false);
		showInspectorOnSelect = false;
		selectionManager = customSelectionManager;
	}

	helpID = "Automation";
}

Automation::~Automation()
{
}

void Automation::reorderItems()
{
	items.sort(Automation::comparator, true);
	BaseManager::reorderItems();
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
	value->clearRange();
	freeRange = true;
}

void Automation::setRange(float minValue, float maxValue)
{
	value->setRange(minValue, maxValue);
	freeRange = !value->hasRange();
	for (auto &k : items)
	{
		if (freeRange) k->value->clearRange();
		else k->value->setRange(value->minimumValue, value->maximumValue);
	}
}

AutomationKey * Automation::getClosestKeyForPos(float pos, int start, int end)
{
	if (items.size() == 0) return nullptr;

	if (start == -1) start = 0;
	if (end == -1) end = items.size() - 1;

	if (pos < items[0]->position->floatValue()) return items[0];
	if (pos > items[items.size() - 1]->position->floatValue()) return items[items.size() - 1];

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

	if (k->position->floatValue() == pos) return k;
	return nullptr;
}

float Automation::getValueForPosition(float pos)
{
	if (items.size() == 0) return 0;
	if (pos <= items[0]->position->floatValue()) return items[0]->value->floatValue();
	else if (pos >= items[items.size() - 1]->position->floatValue()) return items[items.size() - 1]->value->floatValue();

	AutomationKey * k = getClosestKeyForPos(pos);
	if (k == nullptr) return 0;
	return k->getValue(items[items.indexOf(k) + 1], pos);
}

float Automation::getNormalizedValueForPosition(float pos)
{
	return jmap<float>(getValueForPosition(pos), value->minimumValue, value->maximumValue,0 ,1);
}

AutomationKey * Automation::createItem()
{
	AutomationKey * k = new AutomationKey(value->minimumValue, value->maximumValue);
	if(selectionManager != nullptr) k->setSelectionManager(selectionManager);
	k->position->setRange(0, length->floatValue());
	return k;
}

void Automation::addItems(Array<Point<float>> keys, bool removeExistingOverlappingKeys, bool addToUndo, bool autoSmoothCurve)
{
	if(selectionManager != nullptr) selectionManager->setEnabled(false);

	//Array<UndoableAction *> actions;
	//if(removeExistingOverlappingKeys) actions.addArray(getRemoveKeysBetweenAction(keys[0].x, keys[keys.size() - 1].x));
	
	DBG("Add items in Automation");
	removeKeysBetween(keys[0].x, keys[keys.size() - 1].x);

	Array<AutomationKey *> newKeys;

	int autoIndex = items.size();
	for (auto &k : keys)
	{
		AutomationKey * ak = createItem();
		ak->setNiceName("Key " + String(autoIndex));
		ak->position->setValue(k.x);
		ak->value->setValue(k.y);
		if (autoSmoothCurve) ak->setEasing(Easing::BEZIER);
		newKeys.add(ak);

		autoIndex++;
	}

	DBG("Here add items " << newKeys.size() << " items");
	BaseManager::addItems(newKeys);

	if(selectionManager != nullptr) selectionManager->setEnabled(true);
}

void Automation::addItem(const float _position, const float _value, bool addToUndo, bool reorder)
{
	AutomationKey * k = createItem();
	k->position->setValue(_position);
	k->value->setValue(_value);
	BaseManager::addItem(k,var(), addToUndo);
	if (reorder) reorderItems();

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

		value->setValue(getValueForPosition(position->floatValue()));

		if (c == t->position)
		{
			int index = items.indexOf(t);
			bool swapped = true;

			if (index > 0 && t->position->floatValue() < items[index - 1]->position->floatValue()) items.swap(index, index - 1);
			else if (index < items.size() - 1 && t->position->floatValue() > items[index + 1]->position->floatValue()) items.swap(index, index + 1);
			else swapped = false;

			if (swapped)
			{
				baseManagerListeners.call(&Listener::itemsReordered);
				managerNotifier.addMessage(new BaseManager::ManagerEvent(BaseManager::ManagerEvent::ITEMS_REORDERED));
			}
		}
	}
}

void Automation::onContainerParameterChanged(Parameter * p)
{
	if (p == position)
	{
		value->setValue(getValueForPosition(position->floatValue()));
	} else if (p == enableSnap)
	{
		snapSensitivity->setEnabled(enableSnap->boolValue());
	} else if (p == length)
	{
		position->setRange(0, length->floatValue());
		for (auto &k : items) k->position->setRange(0, length->floatValue());
	}
}

InspectableEditor * Automation::getEditor(bool isRoot)
{
	return new AutomationEditor(this, isRoot);
}
