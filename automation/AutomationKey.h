/*
  ==============================================================================

    AutomationKey.h
    Created: 11 Dec 2016 1:22:20pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationKey :
    public BaseItem,
    public Inspectable::InspectableListener
{
public:
    AutomationKey(const float &position = 0, const float &value = 0);
    ~AutomationKey();

    EnumParameter* easingType;
    std::unique_ptr<Easing> easing;
    AutomationKey* nextKey;
    FloatParameter* position;
    FloatParameter* value;

    void setEasing(Easing::Type type);
    void setNextKey(AutomationKey* key);

    float getValueAt(const float& position);
    float getLength() const;

    void setValueRange(float minVal, float maxVal, bool proportional = false);
    void clearValueRange();

    Point<float> getPosAndValue();
    void setPosAndValue(Point<float> posAndValue, bool addToUndo = false);


    //UI manipulation
    void setMovePositionReferenceInternal() override;
    void setPosition(Point<float> targetTime) override;
    Point<float> getPosition() override;
    void addUndoableMoveAction(Array<UndoableAction*>& actions) override;

    void onContainerParameterChangedInternal(Parameter* p) override;
    void onExternalParameterValueChanged(Parameter* p) override;
    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

    void inspectableSelectionChanged(Inspectable* i) override;
    void setSelectedInternal(bool) override;

    void inspectableDestroyed(Inspectable* i) override;

    bool isThisOrChildSelected();

    void updateEasingKeys();
    void notifyKeyUpdated();


    String getTypeString() const override { return "Key"; }

    


    class  AutomationKeyEvent
    {
    public:
        enum Type { KEY_UPDATED, SELECTION_CHANGED };

        AutomationKeyEvent(Type t, AutomationKey* key) : type(t), key(key) {}

        Type type;
        WeakReference<AutomationKey> key;
    };

    QueuedNotifier<AutomationKeyEvent> keyNotifier;
    typedef QueuedNotifier<AutomationKeyEvent>::Listener AsyncListener;


    void addAsyncKeyListener(AsyncListener* newListener) { keyNotifier.addListener(newListener); }
    void addAsyncCoalescedKeyListener(AsyncListener* newListener) { keyNotifier.addAsyncCoalescedListener(newListener); }
    void removeAsyncKeyListener(AsyncListener* listener) { keyNotifier.removeListener(listener); }

private:
    WeakReference<AutomationKey>::Master masterReference;
    friend class WeakReference<AutomationKey>;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationKey)
};

