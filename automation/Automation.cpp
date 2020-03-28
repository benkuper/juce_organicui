/*
  ==============================================================================

    Automation.cpp
    Created: 21 Mar 2020 4:06:01pm
    Author:  bkupe

  ==============================================================================
*/

Automation::Automation(const String& name) :
    BaseManager(name),
    showUIInEditor(true)
{
    isSelectable = false;

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

    valueRange = addPoint2DParameter("Range", "The range between allowed minimum value and maximum value");
    valueRange->canBeDisabledByUser = true;

    viewValueRange = addPoint2DParameter("View Value Range", "The minimum and maximum values to view in editor");
    viewValueRange->hideInEditor = true;
    viewValueRange->setPoint(0, 1);
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

void Automation::addItemInternal(AutomationKey* k, var)
{
    updateCurve();
}

void Automation::removeItemInternal(AutomationKey*)
{
    updateCurve();
}

void Automation::updateCurve()
{
    if (isCurrentlyLoadingData || Engine::mainEngine->isClearing) return;

    Array<float> prevCurvePositions;

    bounds = items.size() > 0 ? items[0]->easing->getBounds() : Rectangle<float>(0, 0, 0, 0);

    float curLength = 0;
    int numItems = items.size();
    for (int i = 0; i < numItems; i++)
    {

        if (i < numItems - 1) items[i]->setNextKey(items[i + 1]);
        if (i < numItems - 1) curLength += items[i]->getLength();

        bounds = bounds.getUnion(items[i]->easing->getBounds());
    }

    computeValue();
}

void Automation::computeValue()
{
    value->setValue(getValueAtPosition(position->floatValue()));
}

void Automation::setLength(float newLength, float stretch, float stickToEnd)
{
    length->setValue(newLength);
}

Point<float> Automation::getPosAndValue()
{
    return Point<float>(position->floatValue(), value->floatValue());
}

void Automation::updateRange()
{
    if (valueRange->enabled)
    {
        value->setRange(valueRange->x, valueRange->y);
        viewValueRange->setRange(valueRange->x, valueRange->y);
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
    if (pos == 0) return items[0];

    for (int i = items.size() - 1; i >= 0; i--)
    {
        if (items[i]->position->floatValue() <= pos) return items[i];
    }

    return nullptr;
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

void Automation::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
    BaseManager::onControllableFeedbackUpdate(cc, c);

    if (AutomationKey* k = dynamic_cast<AutomationKey*>(cc))
    {
        updateCurve();
    }
}

void Automation::afterLoadJSONDataInternal()
{
    updateCurve();
}

InspectableEditor* Automation::getEditor(bool isRoot)
{
    return new AutomationEditor(this, isRoot);
}