#include "Automation.h"
/*
  ==============================================================================

    Automation.cpp
    Created: 21 Mar 2020 4:06:01pm
    Author:  bkupe

  ==============================================================================
*/

Automation::Automation(const String& name, AutomationRecorder * recorder, bool allowKeysOutside) :
    BaseManager(name),
    recorder(recorder),
    allowKeysOutside(allowKeysOutside)
{
    comparator.compareFunc = &Automation::compareKeys;

    editorCanBeCollapsed = false;
    showInspectorOnSelect = false;
    userCanAddItemsManually = false;
    canInspectChildContainers = false;

    length = addFloatParameter("Length", "The length of the curve", 0, 0.01f);
    length->hideInEditor = true;
    length->setControllableFeedbackOnly(true);

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
    valueRange->setPoint(0, 1);

}

Automation::~Automation()
{

}

AutomationKey * Automation::addKey(const float& _position, const float& _value, bool addToUndo)
{
    AutomationKey* key = new AutomationKey(_position, _value);

    var params = new DynamicObject();
    if (AutomationKey* k = getKeyForPosition(_position)) params.getDynamicObject()->setProperty("index", items.indexOf(k) + 1);
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
    else addItems(keysToAdd, var(), false);

    if (addToUndo) UndoMaster::getInstance()->performActions("Add Keys", actions);

}

void Automation::addFromPointsAndSimplify(const Array<Point<float>>& sourcePoints, bool addToUndo, bool removeExistingKeys)
{
    Array<float> points;
    for (auto& pp : sourcePoints) points.add(pp.x, pp.y);
    float* result;
    unsigned int resultNum = 0;
    unsigned int* origIndex;
    unsigned int* corners = nullptr;
    unsigned int cornersLength = 0;
    unsigned int* cornerIndex = nullptr;
    unsigned int cornerIndexLength = 0;

    int detectResult = curve_fit_corners_detect_fl(points.getRawDataPointer(), points.size(), 2, 0, .02f, 20, 30, &corners, &cornersLength);
    if (cornersLength == 0) corners = nullptr;

    int fitResult = curve_fit_cubic_to_points_fl(points.getRawDataPointer(), points.size(), 2, .04f, CURVE_FIT_CALC_HIGH_QUALIY, corners, cornersLength, &result, &resultNum, &origIndex, &cornerIndex, &cornerIndexLength);

    int numPoints = ((int)resultNum);

    Array<AutomationKey*> keys;

    AutomationKey* prevKey = nullptr;
    CubicEasing* prevEasing = nullptr;

    for (int i = 0; i < numPoints; i++)
    {
        int index = i * 6;
        Point<float> h1(result[index + 0], result[index + 1]);
        Point<float> rp(result[index + 2], result[index + 3]);
        Point<float> h2(result[index + 4], result[index + 5]);


        if (prevEasing != nullptr && h1.getDistanceFromOrigin() < 100)
        {
            prevEasing->anchor2->setPoint(h1 - rp);
        }

        if (rp.getDistanceFromOrigin() > 100)
        {
            break;
        }

        AutomationKey* k = new AutomationKey();
        k->setPosAndValue(rp);

        k->easingType->setValueWithData(Easing::BEZIER);
        CubicEasing* ce = (CubicEasing*)k->easing.get();
        if (h2.getDistanceFromOrigin() < 100) ce->anchor1->setPoint(h2 - rp);

        keys.add(k);

        prevEasing = ce;
    }

    delete result;
    delete origIndex;
    delete corners;
    delete cornerIndex;

    addKeys(keys, addToUndo, removeExistingKeys);
}

void Automation::addItemInternal(AutomationKey* k, var)
{
    if (!allowKeysOutside) k->position->setRange(0, length->floatValue());
    if(valueRange->enabled) k->setValueRange(valueRange->x, valueRange->y);

    if(!isManipulatingMultipleItems) updateNextKeys(items.indexOf(k) - 1, items.indexOf(k) + 1);
}

void Automation::addItemsInternal(Array<AutomationKey*>, var params)
{
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

void Automation::updateNextKeys(int start, int end)
{
    if (isCurrentlyLoadingData || Engine::mainEngine->isClearing) return;
    if (items.size() == 0) return;

    int startIndex = jmax(start, 0);
    int endIndex = end == -1 ? items.size() : jmin(end, items.size());

    for (int i = startIndex; i < endIndex; i++)
    {
        if (i < items.size() - 1)
        {
            jassert(items[i]->position->floatValue() <= items[i + 1]->position->floatValue());
            items[i]->setNextKey(items[i + 1]);
        }
    }

    computeValue();
}

void Automation::computeValue()
{
    value->setValue(getValueAtPosition(position->floatValue()));
}

void Automation::setLength(float newLength, float stretch, float stickToEnd)
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

    if (!allowKeysOutside) for (auto& k : items) k->position->setRange(0, length->floatValue());
}

Point<float> Automation::getPosAndValue()
{
    return Point<float>(position->floatValue(), value->floatValue());
}

juce::Rectangle<float> Automation::getBounds()
{
    juce::Rectangle<float> bounds;
    for (int i = 0; i < items.size(); i++)
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
        value->setRange(valueRange->x, valueRange->y);
        viewValueRange->setBounds(valueRange->x, valueRange->x, valueRange->y, valueRange->y);
        for (auto& k : items) k->setValueRange(valueRange->x, valueRange->y);
    }
    else
    {
        value->clearRange();
        viewValueRange->clearRange();
        for (auto& k : items) k->clearValueRange();
    }
}

AutomationKey* Automation::getKeyForPosition(float pos)
{
    if (items.size() == 0) return nullptr;
    if (pos < items[0]->position->floatValue()) return items[0];
    if (pos == 0) return items[0];

    for (int i = items.size() - 1; i >= 0; i--)
    {
        if (items[i]->position->floatValue() <= pos) return items[i];
    }

    return nullptr;
}

Array<AutomationKey*> Automation::getKeysBetweenPositions(float startPos, float endPos)
{
    Array<AutomationKey*> result;
    if (items.size() == 0) return result;
    if(startPos > items[items.size() - 1]->position->floatValue() || endPos < items[0]->position->floatValue()) return result;

    AutomationKey* startK = getKeyForPosition(startPos);
    if (startK->position->floatValue() < startPos) startK = startK->nextKey;

    if (startK == nullptr) return result;

    AutomationKey* endK = getKeyForPosition(endPos);
    if (endK == nullptr) endK = startK;

    int startIndex = items.indexOf(startK);
    int endIndex = items.indexOf(endK)+1;
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
    if (pos >= items[items.size()-1]->position->floatValue())  return items[items.size() - 1]->value->floatValue();

    AutomationKey* k = getKeyForPosition(pos);
    if (k == nullptr || k->easing == nullptr) return 0;
    float normPos = (pos - k->position->floatValue()) / k->getLength();
    return k->easing->getValue(normPos);
}

float Automation::getNormalizedValueAtPosition(float pos)
{
    if (!viewValueRange->enabled) return 0;
    if (items.size() == 0) return 0;
    if (items.size() == 1) return items[0]->value->getNormalizedValue();
    if (pos == length->floatValue())  return items[items.size() - 1]->value->getNormalizedValue();

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


void Automation::onControllableStateChanged(Controllable* c)
{
    if (c == valueRange)
    {
        updateRange();
    }
}

void Automation::afterLoadJSONDataInternal()
{
    updateNextKeys();
}

int Automation::compareKeys(AutomationKey* k1, AutomationKey* k2)
{
    return k2->position->floatValue() < k1->position->floatValue() ? 1 : k2->position->floatValue() > k1->position->floatValue() ? -1 : 0;
}

InspectableEditor* Automation::getEditor(bool isRoot)
{
    return new AutomationEditor(this, isRoot);
}