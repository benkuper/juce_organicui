/*
  ==============================================================================

	Automation.cpp
	Created: 21 Mar 2020 4:06:01pm
	Author:  bkupe

  ==============================================================================
*/

Automation::Automation(const String& name, AutomationRecorder* recorder, bool allowKeysOutside) :
	BaseManager(name),
	allowKeysOutside(allowKeysOutside),
	recorder(recorder),
	positionUnitSteps(0),
	automationNotifier(5)
{
	itemDataType = "AutomationKey";

	comparator.compareFunc = &Automation::compareKeys;

	editorCanBeCollapsed = false;
	showInspectorOnSelect = false;
	//userCanAddItemsManually = false;
	canInspectChildContainers = false;

	length = addFloatParameter("Length", "The length of the curve", 1, 0.01f);
	length->hideInEditor = true;
	length->setControllableFeedbackOnly(true);
	length->forceSaveValue = true;

	position = addFloatParameter("Position", "The length of the curve", 0);
	position->hideInEditor = true;

	value = addFloatParameter("Value", "The current value of the curve at the current position", 0, 0, 1);
	value->hideInEditor = true;
	value->setControllableFeedbackOnly(true);

	viewValueRange = addPoint2DParameter("View Value Range", "The minimum and maximum values to view in editor");
	viewValueRange->hideInEditor = true;
	viewValueRange->setPoint(0, 1);

	valueRange = addPoint2DParameter("Range", "The range between allowed minimum value and maximum value");
	valueRange->canBeDisabledByUser = true;
	valueRange->hideInEditor = true;

	rangeRemapMode = addEnumParameter("Range Remap Mode", "The way of recaculating the key values when changing the range.\nAbsolute means no modification is done. Proportional means that the relative value of the key will be maintained.");
	rangeRemapMode->addOption("Absolute", ABSOLUTE)->addOption("Proportional", PROPORTIONAL);
	rangeRemapMode->hideInEditor = true;
	valueRange->setPoint(0, 1);

	scriptObject.setMethod("getValueAtPosition", &Automation::getValueAtPositionFromScript);
	scriptObject.setMethod("getKeyAtPosition", &Automation::getKeyAtPositionFromScript);
	scriptObject.setMethod("getKeysBetween", &Automation::getKeysBetweenFromScript);
}

Automation::~Automation()
{

}

AutomationKey* Automation::addKey(const float& _position, const float& _value, bool addToUndo)
{
	AutomationKey* key = new AutomationKey(_position, _value);
	var params = new DynamicObject();
	if (items.size() > 0 && items[0]->position->floatValue() > _position) params.getDynamicObject()->setProperty("index", 0);
	else if (AutomationKey* k = getKeyForPosition(_position)) params.getDynamicObject()->setProperty("index", items.indexOf(k) + 1);
	return addItem(key, params, addToUndo);
}

void Automation::addKeys(const Array<AutomationKey*>& keys, bool addToUndo, bool removeExistingKeys)
{
	if (keys.size() == 0) return;

	Array<UndoableAction*> actions;

	if (removeExistingKeys)
	{
		Array<AutomationKey*> existingKeys = getKeysBetweenPositions(keys[0]->position->floatValue(), keys[keys.size() - 1]->position->floatValue());
		if (addToUndo) actions.addArray(getRemoveItemsUndoableAction(existingKeys));
		else removeItems(existingKeys, false);
	}

	Array<AutomationKey*> keysToAdd;
	if (addToUndo)  actions.add(getAddItemsUndoableAction(keys));
	else addItems(keys, var(), false);

	if (addToUndo) UndoMaster::getInstance()->performActions("Add Keys", actions);

}

void Automation::insertKeyAt(const float& position, bool addToUndo)
{
	AutomationKey* startKey = getKeyForPosition(position);// ((AutomationKeyUI*)eui->getParentComponent())->item;
	if (startKey == nullptr) return;

	Point<float> p = startKey->easing->getClosestPointForPos(position);

	Array<Point<float>> controlPoints;
	if (startKey->easing->type == Easing::BEZIER)
	{
		CubicEasing* ce1 = (CubicEasing*)startKey->easing.get();
		controlPoints = ce1->getSplitControlPoints(p.x);
	}

	AutomationKey* k = addKey(p.x, p.y, addToUndo);
	k->easingType->setValueWithData(startKey->easingType->getValueData());

	if (startKey->easing->type == Easing::BEZIER)
	{
		CubicEasing* ce1 = (CubicEasing*)startKey->easing.get();
		CubicEasing* ce2 = (CubicEasing*)k->easing.get();

		ce1->anchor1->setPoint(controlPoints[0] - ce1->start);
		ce1->anchor2->setPoint(controlPoints[1] - ce1->end);

		ce2->anchor1->setPoint(controlPoints[2] - ce2->start);
		ce2->anchor2->setPoint(controlPoints[3] - ce2->end);
	}
}

void Automation::addFromPointsAndSimplifyBezier(const Array<Point<float>>& sourcePoints, bool addToUndo, bool removeExistingKeys)
{
	if (sourcePoints.size() == 0)
		return;

	// Normalize X and Y values to [0,1] for correct corner detection
	float xScale = sourcePoints.getLast().x - sourcePoints.getLast().y;
	float yScale = valueRange->enabled ? (valueRange->y - valueRange->x) : 1.0f;

	Array<float> points;
	for (auto& pp : sourcePoints)
		points.add(pp.x / xScale, pp.y / yScale);

	float* result;
	unsigned int resultNum = 0;
	unsigned int* corners = nullptr;
	unsigned int cornersLength = 0;
	unsigned int* cornerIndex = nullptr;
	unsigned int cornerIndexLength = 0;

	const float errorThreshold = 0.03f;// 0.02f;

	curve_fit_corners_detect_fl(points.getRawDataPointer(), points.size() / 2, 2,
		errorThreshold / 4, errorThreshold * 4, 32, M_PI / 8,
		&corners, &cornersLength);
	if (cornersLength == 0) corners = nullptr;

	curve_fit_cubic_to_points_fl(points.getRawDataPointer(), points.size() / 2, 2,
		errorThreshold, CURVE_FIT_CALC_HIGH_QUALIY,
		corners, cornersLength,
		&result, &resultNum,
		nullptr,
		&cornerIndex, &cornerIndexLength);

	int numPoints = ((int)resultNum);

	Array<AutomationKey*> keys;

	CubicEasing* prevEasing = nullptr;
	Point<float> prevRP;

	float maxDist = valueRange->enabled ? (valueRange->y - valueRange->x) * 100 : 1000;

	int numBadPoints = 0;
	for (int i = 0; i < numPoints; ++i)
	{
		int index = i * 6;
		Point<float> h1(result[index + 0] * xScale, result[index + 1] * yScale);
		Point<float> rp(result[index + 2] * xScale, result[index + 3] * yScale);
		Point<float> h2(result[index + 4] * xScale, result[index + 5] * yScale);


		if (prevEasing != nullptr && h1.getDistanceFrom(rp) < maxDist)
		{
			prevEasing->anchor2->setPoint(h1 - rp);
		}


		if (i > 0 && (rp.getDistanceFrom(prevRP) > maxDist || rp.x <= prevRP.x))
		{
			numBadPoints++;
			continue;
		}

		AutomationKey* k = new AutomationKey();
		k->setPosAndValue(rp);
		k->setNiceName("Key " + String(i));
		k->easingType->setValueWithData(Easing::BEZIER);
		CubicEasing* ce = (CubicEasing*)k->easing.get();
		if (h2.getDistanceFrom(rp) < maxDist) ce->anchor1->setPoint(h2 - rp);

		keys.add(k);

		prevEasing = ce;
		prevRP.setXY(rp.x, rp.y);
	}

	free(result);
	free(corners);
	free(cornerIndex);

	addKeys(keys, addToUndo, removeExistingKeys);
}

void Automation::addFromPointsAndSimplifyLinear(const Array<Point<float>>& sourcePoints, float tolerance, bool addToUndo, bool removeExistingKeys)
{
	Array<Point<float>> simplifiedPoints = getLinearSimplifiedPointsFrom(sourcePoints, tolerance);
	Array<AutomationKey*> keys;

	for (auto& p : simplifiedPoints)
	{
		AutomationKey* k = new AutomationKey();
		k->setPosAndValue(p);
		k->easingType->setValueWithData(Easing::LINEAR);
		keys.add(k);
	}

	addKeys(keys, addToUndo, removeExistingKeys);
}

Array<Point<float>> Automation::getLinearSimplifiedPointsFrom(const Array<Point<float>>& sourcePoints, float tolerance, int start, int end)
{
	if (sourcePoints.size() < 2) return sourcePoints;

	// Find the point with the maximum distance from line between start and end
	double maxDist = 0.0;
	size_t index = 0;
	if (end == -1)  end = sourcePoints.size() - 1;

	Point<float> startP = sourcePoints[start];
	Point<float> endP = sourcePoints[end];
	Line<float> line(startP, endP);

	for (size_t i = start + 1; i < end - 1; ++i)
	{
		Point<float> pol;
		float dist = line.getDistanceFromPoint(sourcePoints[(int)i], pol);
		if (dist > maxDist)
		{
			index = i;
			maxDist = dist;
		}
	}

	Array<Point<float>> result;

	//If there is a range, we multiply the factor by the amplitude of this range, so 1 is the max value difference
	float mappedTolerance = tolerance;
	if (valueRange->enabled) mappedTolerance = tolerance * (valueRange->y - valueRange->x);

	// If max distance is greater than epsilon, recursively simplify
	if (maxDist > mappedTolerance)
	{
		// Recursive call
		int start1 = start;
		int end1 = (int)index;
		int start2 = (int)index;
		int end2 = end;

		Array<Point<float>> result1 = getLinearSimplifiedPointsFrom(sourcePoints, tolerance, start1, end1);
		Array<Point<float>> result2 = getLinearSimplifiedPointsFrom(sourcePoints, tolerance, start2, end2);
		result1.removeLast();
		// Build the result list
		result.addArray(result1);
		result.addArray(result2);
	}
	else
	{
		result.add(startP);
		result.add(endP);
	}

	return result;
}

void Automation::launchInteractiveSimplification(const Array<Point<float>>& sourcePoints)
{
	if (recorder == nullptr) return;

	recorder->simplificationTolerance->addParameterListener(this);

	interactiveSourcePoints = sourcePoints;
	interactiveSimplifiedPoints = getLinearSimplifiedPointsFrom(sourcePoints, recorder->simplificationTolerance->floatValue());
	automationNotifier.addMessage(new AutomationEvent(AutomationEvent::INTERACTIVE_SIMPLIFICATION_CHANGED, this));
}

void Automation::finishInteractiveSimplification()
{
	recorder->simplificationTolerance->removeParameterListener(this);

	Array<AutomationKey*> keys;
	for (auto& p : interactiveSimplifiedPoints)
	{
		AutomationKey* k = new AutomationKey();
		k->setPosAndValue(p);
		k->easingType->setValueWithData(Easing::LINEAR);
		keys.add(k);
	}

	interactiveSourcePoints.clear();
	interactiveSimplifiedPoints.clear();

	automationNotifier.addMessage(new AutomationEvent(AutomationEvent::INTERACTIVE_SIMPLIFICATION_CHANGED, this));
	addKeys(keys, true, true);
}

void Automation::setUnitSteps(float unitSteps)
{
	if (positionUnitSteps == unitSteps) return;
	positionUnitSteps = jmax<float>(unitSteps, 0);

	for (auto& i : items)
	{
		i->position->unitSteps = positionUnitSteps;
		if (positionUnitSteps > 0) i->position->setValue(i->position->getStepSnappedValueFor(i->position->floatValue()));
	}
}

void Automation::addItemInternal(AutomationKey* k, var)
{
	k->position->unitSteps = positionUnitSteps;
	if (positionUnitSteps > 0) k->position->setValue(k->position->getStepSnappedValueFor(k->position->floatValue()));

	if (!allowKeysOutside) k->position->setRange(0, length->floatValue());

	if (valueRange->enabled) k->setValueRange(valueRange->x, valueRange->y);

	if (!isManipulatingMultipleItems) updateNextKeys(items.indexOf(k) - 1, items.indexOf(k) + 1);
}

void Automation::addItemsInternal(Array<AutomationKey*> keys, var params)
{
	for (auto& k : keys) k->position->unitSteps = positionUnitSteps;
	updateNextKeys();
}

void Automation::removeItemInternal(AutomationKey* k)
{
	if (!isManipulatingMultipleItems) updateNextKeys();
}

void Automation::removeItemsInternal()
{
	updateNextKeys();
}

Array<AutomationKey*> Automation::addItemsFromClipboard(bool showWarning)
{
	Array<AutomationKey*> keys = BaseManager::addItemsFromClipboard(showWarning);

	if (keys.isEmpty()) return keys;
	if (keys[0] == nullptr) return Array<AutomationKey*>();

	float minTime = keys[0]->position->floatValue();
	for (auto& b : keys)
	{
		if (b->position->floatValue() < minTime)
		{
			minTime = b->position->floatValue();
		}
	}

	float diffTime = position->floatValue() - minTime;

	if (keys.size() > 1)
	{
		Array<AutomationKey*> keysBetween = getKeysBetweenPositions(keys[0]->position->floatValue() + diffTime, keys[keys.size() - 1]->position->floatValue() + diffTime);
		if (keysBetween.size() > 0)
		{
			Array<AutomationKey*> keysToRemove;
			for (auto& k : keysBetween) if (!keys.contains(k)) keysToRemove.add(k);
			removeItems(keysToRemove);
		}
	}

	for (auto& k : keys) k->position->setValue(k->position->floatValue() + diffTime);

	reorderItems();

	return keys;
}

Array<UndoableAction*> Automation::getMoveKeysBy(float start, float offset)
{
	Array<UndoableAction*> actions;

	if (items.size() == 0) return actions;

	AutomationKey* k = getKeyForPosition(start);
	if (k->position->floatValue() < start) k = k->nextKey;
	if (k == nullptr) return Array<UndoableAction*>();

	while (k != nullptr)
	{
		actions.add(k->position->setUndoableValue(k->position->floatValue(), k->position->floatValue() + offset, true));
		k = k->nextKey;
	}

	return actions;
}

Array<UndoableAction*> Automation::getRemoveTimespan(float start, float end)
{
	Array<UndoableAction*> actions;

	Array<AutomationKey*> keys = getKeysBetweenPositions(start, end);
	actions.addArray(getRemoveItemsUndoableAction(keys));
	actions.addArray(getMoveKeysBy(end, start - end));

	return actions;
}

void Automation::updateNextKeys(int start, int end)
{
	if (isCurrentlyLoadingData || Engine::mainEngine->isClearing) return;
	if (items.size() == 0) return;

	int startIndex = jmax(start, 0);
	int endIndex = end == -1 ? items.size() : jmin(end, items.size());

	for (int i = startIndex; i < endIndex; ++i)
	{
		if (i < items.size() - 1)
		{
			jassert(items[i]->position->floatValue() <= items[i + 1]->position->floatValue());
			items[i]->setNextKey(items[i + 1]);
		}
		else
		{
			items[i]->setNextKey(nullptr);
		}
	}

	computeValue();
}

void Automation::computeValue()
{
	value->setValue(getValueAtPosition(position->floatValue()));
}

void Automation::setLength(float newLength, bool stretch, bool stickToEnd)
{
	if (length->floatValue() == newLength) return;

	if (stretch && length->floatValue() > 0)
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

	if (!allowKeysOutside)
	{
		for (auto& k : items)
		{
			k->position->setRange(0, length->floatValue());
		}
	}
}

Point<float> Automation::getPosAndValue()
{
	return Point<float>(position->floatValue(), value->floatValue());
}

juce::Rectangle<float> Automation::getBounds()
{
	juce::Rectangle<float> bounds;
	for (int i = 0; i < items.size(); ++i)
	{
		if (i < items.size() - 1) items[i]->setNextKey(items[i + 1]);
		bounds = bounds.getUnion(items[i]->easing->getBounds());
	}

	return bounds;
}

void Automation::updateRange()
{
	if (valueRange->enabled)
	{
		if (valueRange->y < valueRange->x + .2f)
		{
			valueRange->setPoint(valueRange->x, jmax(valueRange->y, valueRange->x + .2f));
			return;
		}

		value->setRange(valueRange->x, valueRange->y);
		viewValueRange->setBounds(valueRange->x, valueRange->x, valueRange->y, valueRange->y);
		viewValueRange->setPoint(valueRange->getPoint());

		RangeRemapMode rrm = rangeRemapMode->getValueDataAsEnum<RangeRemapMode>();
		for (auto& k : items) k->setValueRange(valueRange->x, valueRange->y, rrm == PROPORTIONAL);
	}
	else
	{
		value->clearRange();
		viewValueRange->clearRange();
		for (auto& k : items) k->clearValueRange();
	}
}

AutomationKey* Automation::getKeyForPosition(float pos, bool trueIfEqual)
{
	if (items.size() == 0) return nullptr;
	if (pos < items[0]->position->floatValue()) return items[0];
	if (pos == 0) return items[0];

	for (int i = items.size() - 1; i >= 0; i--)
	{
		float p = items[i]->position->floatValue();
		if (p < pos || (p == pos && trueIfEqual)) return items[i];
	}

	return nullptr;
}

AutomationKey* Automation::getNextKeyForPosition(float pos, bool trueIfEqual)
{
	if (items.size() == 0) return nullptr;
	if (pos < items[0]->position->floatValue()) return items[0];
	if (pos == 0) return items[0];

	for (int i = 0; i < items.size(); i++)
	{
		float p = items[i]->position->floatValue();
		if (p > pos || (p == pos && trueIfEqual)) return items[i];
	}

	return nullptr;
}

Array<AutomationKey*> Automation::getKeysBetweenPositions(float startPos, float endPos)
{
	Array<AutomationKey*> result;
	if (items.size() == 0) return result;
	if (startPos > items[items.size() - 1]->position->floatValue() || endPos < items[0]->position->floatValue()) return result;

	AutomationKey* startK = getKeyForPosition(startPos);
	if (startK->position->floatValue() < startPos) startK = startK->nextKey;

	if (startK == nullptr) return result;

	AutomationKey* endK = getKeyForPosition(endPos);
	if (endK == nullptr) endK = startK;

	int startIndex = items.indexOf(startK);
	int endIndex = items.indexOf(endK) + 1;
	result.addArray(items, startIndex, endIndex - startIndex);

	return result;
}

float Automation::getValueAtNormalizedPosition(float pos)
{
	return getValueAtPosition(pos * length->floatValue());
}


float Automation::getValueAtPosition(float pos)
{
	if (items.size() == 0) return 0;
	if (items.size() == 1) return items[0]->value->floatValue();
	if (pos <= items[0]->position->floatValue()) return items[0]->value->floatValue();
	if (pos >= items[items.size() - 1]->position->floatValue())  return items[items.size() - 1]->value->floatValue();

	AutomationKey* k = getKeyForPosition(pos);
	if (k == nullptr || k->easing == nullptr) return 0;
	float normPos = (pos - k->position->floatValue()) / k->getLength();
	return k->easing->getValue(normPos);
}

float Automation::getNormalizedValueAtPosition(float pos)
{
	if (!viewValueRange->enabled) return 0;
	if (items.size() == 0) return 0;
	if (items.size() == 1) return (float)items[0]->value->getNormalizedValue();
	if (pos == length->floatValue())  return (float)items[items.size() - 1]->value->getNormalizedValue();

	AutomationKey* k = getKeyForPosition(pos);
	if (k == nullptr || k->easing == nullptr) return 0;
	float normPos = (pos - k->position->floatValue()) / k->getLength();
	return jmap<float>(k->easing->getValue(normPos), valueRange->x, valueRange->y, 0, 1);
}

void Automation::onContainerParameterChanged(Parameter* p)
{
	BaseManager::onContainerParameterChanged(p);
	if (p == position)
	{
		computeValue();
	}
	else if (p == valueRange)
	{
		updateRange();
	}
	else if (p == length)
	{
		position->setRange(0, length->floatValue());
	}
}

void Automation::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	if (AutomationKey* k = dynamic_cast<AutomationKey*>(cc))
	{
		if (c == k->value || c == k->position || c->parentContainer == k->easing.get())
		{
			computeValue();
		}
	}

}


void Automation::onControllableStateChanged(Controllable* c)
{
	if (c == valueRange)
	{
		updateRange();
	}
}

void Automation::onExternalParameterValueChanged(Parameter* p)
{
	BaseManager::onExternalParameterValueChanged(p);
	if (recorder != nullptr && p == recorder->simplificationTolerance)
	{
		if (!interactiveSourcePoints.isEmpty())
		{
			interactiveSimplifiedPoints = getLinearSimplifiedPointsFrom(interactiveSourcePoints, recorder->simplificationTolerance->floatValue());
			automationNotifier.addMessage(new AutomationEvent(AutomationEvent::INTERACTIVE_SIMPLIFICATION_CHANGED, this));
		}
	}
}

void Automation::reorderItems()
{
	BaseManager::reorderItems();
	updateNextKeys();
}

void Automation::afterLoadJSONDataInternal()
{
	reorderItems();
}

int Automation::compareKeys(AutomationKey* k1, AutomationKey* k2)
{
	return k2->position->floatValue() < k1->position->floatValue() ? 1 : k2->position->floatValue() > k1->position->floatValue() ? -1 : 0;
}

var Automation::getValueAtPositionFromScript(const juce::var::NativeFunctionArgs& a)
{
	Automation* au = getObjectFromJS<Automation>(a);
	if (!checkNumArgs(au->niceName, a, 1)) return var();
	return au->getValueAtPosition(a.arguments[0]);
}

var Automation::getKeyAtPositionFromScript(const juce::var::NativeFunctionArgs& a)
{
	Automation* au = getObjectFromJS<Automation>(a);
	if (!checkNumArgs(au->niceName, a, 1)) return var();
	return au->getKeyForPosition(a.arguments[0])->getScriptObject();
}

var Automation::getKeysBetweenFromScript(const juce::var::NativeFunctionArgs& a)
{
	Automation* au = getObjectFromJS<Automation>(a);
	if (!checkNumArgs(au->niceName, a, 2)) return var();
	var result = var();
	Array<AutomationKey* > keys = au->getKeysBetweenPositions(a.arguments[0], a.arguments[1]);
	for (auto& k : keys) result.append(k->getScriptObject());
	return result;
}

InspectableEditor* Automation::getEditorInternal(bool isRoot)
{
	return new AutomationEditor(this, isRoot);
}
