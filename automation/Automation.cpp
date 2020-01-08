#include "Automation.h"
/*  ==============================================================================

	Automation.cpp
	Created: 11 Dec 2016 1:21:37pm
	Author:  Ben

  ==============================================================================
*/

AutomationBase::AutomationBase(const String &name) :
	BaseManager(name),
	numDimensions(0),
	value(nullptr),
	recorder(nullptr),
	showUIInEditor(false),
	allowKeysOutside(allowKeysOutside)
{

	notifyStructureChangeWhenLoadingData = false;
		
	comparator.compareFunc = &AutomationBase::compareTime;

	itemDataType = "AutomationKey"; 
	editorCanBeCollapsed = false;

	length = addFloatParameter("Length", "Length of the automation", 1, .1f, INT32_MAX);
	length->defaultUI = FloatParameter::TIME;
	length->hideInEditor = true;

	position = addFloatParameter("Position", "The current position in the automation. Used for automatic retrieve value and feedback.", 0, 0, length->floatValue());
	position->hideInEditor = true;



	enableSnap = addBoolParameter("Enable Snap", "If enabled, moving keys will be automatically adjusted to interesting positions such as automation position", false);
	snapSensitivity = addFloatParameter("Snap Sensitivity", "Controls the sensitivity of the snapping, if enabled.\nThe greater the value, the more likely a position will be snapped.", .5f, 0, 3, false);
	enableSnap->hideInEditor = true;
	snapSensitivity->hideInEditor = true; 

	//selectItemWhenCreated = false;
	
	helpID = "Automation";
}

AutomationBase::~AutomationBase()
{
}


bool AutomationBase::hasRange()
{
	return value != nullptr && value->hasRange();
}

void AutomationBase::setDedicatedSelectionManager(bool value)
{
	if (value) customSelectionManager.reset(new InspectableSelectionManager(false));
	else customSelectionManager.reset();

	showInspectorOnSelect = !value;
	setSelectionManager(value ? customSelectionManager.get():nullptr);

	for (auto& k : items) k->setSelectionManager(value ? customSelectionManager.get() : nullptr);
}

void AutomationBase::setLength(float newLength, bool stretch, bool stickToEnd)
{
	if (length->floatValue() == newLength) return;


	if (stretch)
	{
		float stretchFactor = newLength / length->floatValue();
		if (stretchFactor > 1) length->setValue(newLength); //if stretching, we have first to expand the length in case keys are not allowed outside
		for (auto& k : items) k->position->setValue(k->position->floatValue() * stretchFactor);
		if (stretchFactor < 1) length->setValue(newLength); //if reducing, we have to first reduce keys and then we can reduce the length, in case keys are not allowed outside
	}
	else
	{
		float lengthDiff = newLength - length->floatValue();

		length->setValue(newLength); // just change the value, nothing unusual

		if (stickToEnd) for (auto& k : items) k->position->setValue(k->position->floatValue() + lengthDiff);
	}


}


AutomationKeyBase * AutomationBase::getClosestKeyForPos(float pos, int start, int end)
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
	}
	else
	{
		return getClosestKeyForPos(pos, start, midIndex);
	}
}

AutomationKeyBase * AutomationBase::getKeyAtPos(float pos)
{
	AutomationKeyBase * k = getClosestKeyForPos(pos);
	if (k == nullptr) return nullptr;
	if (k->position->floatValue() == pos) return k;
	return nullptr;
}


void AutomationBase::removeKeysBetween(float start, float end)
{

	Array<AutomationKeyBase *> keysToRemove;
	for (auto &k : items) if (k->position->floatValue() >= start && k->position->floatValue() <= end) keysToRemove.add(k);

	removeItems(keysToRemove);
}

void AutomationBase::removeAllSelectedKeys()
{
	Array<AutomationKeyBase*> keysToRemove;
	for (auto &k : items) if (k->isSelected) keysToRemove.add(k);
	for (auto &k : keysToRemove) removeItem(k);
}

void AutomationBase::setSnapPositions(Array<float> positions)
{
	snapPositions = positions;
}

float AutomationBase::getClosestSnapForPos(float pos, int start, int end)
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

void AutomationBase::onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
	AutomationKeyBase * t = dynamic_cast<AutomationKeyBase *>(cc);

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

		updateCurrentValue();

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

void AutomationBase::onContainerParameterChanged(Parameter * p)
{
	if (p == position)
	{
		updateCurrentValue();
	} else if (p == enableSnap)
	{
		snapSensitivity->setEnabled(enableSnap->boolValue());
	} else if (p == length)
	{
		position->setRange(0, length->floatValue());
		if(!allowKeysOutside) for (auto &k : items) k->position->setRange(0, length->floatValue());
	}
}

int AutomationBase::compareTime(AutomationKeyBase * t1, AutomationKeyBase * t2)
{
	if (t1->position->floatValue() < t2->position->floatValue()) return -1;
	else if (t1->position->floatValue() > t2->position->floatValue()) return 1;
	return 0;
}

InspectableEditor * AutomationBase::getEditor(bool isRoot)
{
	return new AutomationTimelineEditor(this, isRoot);
}


