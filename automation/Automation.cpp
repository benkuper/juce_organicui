/*  ==============================================================================

	Automation.cpp
	Created: 11 Dec 2016 1:21:37pm
	Author:  Ben

  ==============================================================================
*/

Automation::Automation(const String &name, AutomationRecorder * _recorder, bool freeRange, bool allowKeysOutside, bool dedicatedSelectionManager) :
	BaseManager(name),
	recorder(_recorder),
	showUIInEditor(false),
	freeRange(freeRange),
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
	if(!allowKeysOutside) k->position->setRange(0, length->floatValue());
	return k;
}

void Automation::addItems(Array<Point<float>> keys, bool removeExistingOverlappingKeys, bool addToUndo, Easing::Type defaultEasing)
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
		ak->setEasing(defaultEasing);
		newKeys.add(ak);

		autoIndex++;
	}

	BaseManager::addItems(newKeys);

	if(selectionManager != nullptr) selectionManager->setEnabled(true);


}

AutomationKey * Automation::addItem(const float _position, const float _value, bool addToUndo, bool reorder)
{
	AutomationKey * k = createItem();
	k->position->setValue(_position);
	k->value->setValue(_value);
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
		value->setValue(getValueForPosition(position->floatValue()));
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
