/*
  ==============================================================================

    AutomationKey.cpp
    Created: 21 Mar 2020 4:06:25pm
    Author:  bkupe

  ==============================================================================
*/

AutomationKey::AutomationKey(const float & _position, const float & _value) :
    BaseItem(getTypeString(), false, false),
    easing(nullptr),
    nextKey(nullptr),
    keyNotifier(8)
{
    itemDataType = "AutomationKey";
    
    saveAndLoadRecursiveData = true;
    showInspectorOnSelect = false;

    position = addFloatParameter("Position", "The position of the key", 0);
    value = addFloatParameter("Value", "The value of the key", 0);

    position->setValue(_position);
    position->resetLastUndoValue();
    position->defaultUI = FloatParameter::TIME;
    value->setValue(_value);
    value->resetLastUndoValue();

    easingType = addEnumParameter("Easing Type", "The type of interpolation to use");
    for (int i = 0; i < Easing::TYPE_MAX; i++) easingType->addOption(Easing::typeNames[i], (Easing::Type)i, false);
    easingType->setValueWithData(GlobalSettings::getInstance()->defaultEasing->getValueDataAsEnum<Easing::Type>());

}

AutomationKey::~AutomationKey()
{
    setNextKey(nullptr);
    if (easing != nullptr) easing->removeInspectableListener(this);
    easing.reset();
    masterReference.clear();
}

void AutomationKey::setEasing(Easing::Type type)
{
    if (easing != nullptr)
    {
        if (easing->type == type) return;
        easing->removeInspectableListener(this);
        removeChildControllableContainer(easing.get());
    }

    Easing* e = nullptr;
    switch (type)
    {
    case Easing::LINEAR: e = new LinearEasing(); break;
    case Easing::BEZIER: e = new CubicEasing(); break;
    case Easing::HOLD: e = new HoldEasing(); break;
    case Easing::SINE: e = new SineEasing(); break;
    case Easing::ELASTIC: e = new ElasticEasing(); break;
    case Easing::BOUNCE: e = new BounceEasing(); break;
    case Easing::STEPS: e = new StepEasing(); break;
    case Easing::NOISE: e = new NoiseEasing(); break;
    case Easing::PERLIN: e = new PerlinEasing(); break;
    default:
        break;
    }

    easing.reset(e);

    if (e != nullptr)
    {
        easing->addInspectableListener(this);
        addChildControllableContainer(easing.get());
    }

    updateEasingKeys();
}

void AutomationKey::setNextKey(AutomationKey* key)
{
    if (nextKey == key) return;

    if (nextKey != nullptr && !nextKey->isClearing)
    {
        nextKey->position->removeParameterListener(this);
        nextKey->value->removeParameterListener(this);
        nextKey->removeInspectableListener(this);

    }

    nextKey = key;

    if (nextKey != nullptr)
    {
        nextKey->position->addParameterListener(this);
        nextKey->value->addParameterListener(this);
        nextKey->addInspectableListener(this);
    }

    updateEasingKeys();

}

float AutomationKey::getValueAt(const float& _position)
{
    if (easing == nullptr && easing->length > 0) return value->floatValue();
    return easing->getValue(_position / easing->length);
}

float AutomationKey::getLength() const
{
    return easing != nullptr ? easing->length : 0;
}

void AutomationKey::setValueRange(float minVal, float maxVal, bool proportional)
{
    float nv = (proportional && value->hasRange()) ? (float)value->getNormalizedValue() : -1;
    value->setRange(minVal, maxVal);
    if (nv != -1) value->setNormalizedValue(nv);
}

void AutomationKey::clearValueRange()
{
    value->clearRange();
}

Point<float> AutomationKey::getPosAndValue()
{
    return Point<float>(position->floatValue(), value->floatValue());
}

void AutomationKey::setPosAndValue(Point<float> posAndValue, bool addToUndo)
{
    if (addToUndo)
    {
        Array<UndoableAction*> actions;
        actions.addArray(position->setUndoableValue(posAndValue.x, true));
        actions.addArray(value->setUndoableValue(posAndValue.y, true));
        UndoMaster::getInstance()->performActions("Set key position and value", actions);
    }
    else
    {
        position->setValue(posAndValue.x);
        value->setValue(posAndValue.y);
    }
}


void AutomationKey::setMovePositionReferenceInternal()
{
    movePositionReference = Point<float>(position->floatValue(), value->floatValue());
}

void AutomationKey::setPosition(Point<float> targetPosition)
{
    setPosAndValue(targetPosition);
}

Point<float> AutomationKey::getPosition()
{
    return getPosAndValue();
}

void AutomationKey::addUndoableMoveAction(Array<UndoableAction*>& actions)
{
    actions.addArray(position->setUndoableValue(position->floatValue(), true));
    actions.addArray(value->setUndoableValue(value->floatValue(), true));
}

void AutomationKey::onContainerParameterChangedInternal(Parameter* p)
{
    BaseItem::onContainerParameterChangedInternal(p);
    if (p == easingType)
    {
        setEasing(easingType->getValueDataAsEnum<Easing::Type>());
    }
    else if (p == position || p == value)
    {
        updateEasingKeys();
    }
}

void AutomationKey::onExternalParameterValueChanged(Parameter* p)
{
    BaseItem::onExternalParameterValueChanged(p);
    if (nextKey != nullptr && (p == nextKey->position || p == nextKey->value))
    {
        updateEasingKeys();
    }
}

void AutomationKey::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
    BaseItem::onControllableFeedbackUpdateInternal(cc, c);
    if (cc == easing.get()) notifyKeyUpdated();
}

void AutomationKey::inspectableSelectionChanged(Inspectable* i)
{
    if (Engine::mainEngine->isClearing || isClearing) return;
    keyNotifier.addMessage(new AutomationKeyEvent(AutomationKeyEvent::SELECTION_CHANGED, this));
}

void AutomationKey::setSelectedInternal(bool)
{
    if (Engine::mainEngine->isClearing || isClearing) return;
    keyNotifier.addMessage(new AutomationKeyEvent(AutomationKeyEvent::SELECTION_CHANGED, this));
}

void AutomationKey::inspectableDestroyed(Inspectable* i)
{
    if (i == nextKey) setNextKey(nullptr);
}

bool AutomationKey::isThisOrChildSelected()
{
    return (isSelected || (easing != nullptr && easing->isSelected));
}

void AutomationKey::updateEasingKeys()
{
    if (isClearing) return;
    if (easing != nullptr)
    {
        easing->updateKeys(getPosAndValue(), nextKey != nullptr ? nextKey->getPosAndValue(): getPosAndValue());
    }

    notifyKeyUpdated();
}

void AutomationKey::notifyKeyUpdated()
{
    keyNotifier.addMessage(new AutomationKeyEvent(AutomationKeyEvent::KEY_UPDATED, this));
}


IMPLEMENT_UI_FUNC;