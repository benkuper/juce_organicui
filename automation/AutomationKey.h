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

    juce::Point<float> getPosAndValue();
    void setPosAndValue(juce::Point<float> posAndValue, bool addToUndo = false);


    //UI manipulation
    void setMovePositionReferenceInternal() override;
    void setPosition(juce::Point<float> targetTime) override;
    juce::Point<float> getPosition() override;
    void addUndoableMoveAction(juce::Array<juce::UndoableAction*>& actions) override;

    void onContainerParameterChangedInternal(Parameter* p) override;
    void onExternalParameterValueChanged(Parameter* p) override;
    void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

    void inspectableSelectionChanged(Inspectable* i) override;
    void setSelectedInternal(bool) override;

    void inspectableDestroyed(Inspectable* i) override;

    bool isThisOrChildSelected();

    void updateEasingKeys();
    void notifyKeyUpdated();


    juce::String getTypeString() const override { return "Key"; }

    DECLARE_UI_FUNC;
    


    class  AutomationKeyEvent
    {
    public:
        enum Type { KEY_UPDATED, SELECTION_CHANGED };

        AutomationKeyEvent(Type t, AutomationKey* key) : type(t), key(key) {}

        Type type;
        juce::WeakReference<AutomationKey> key;
    };

    QueuedNotifier<AutomationKeyEvent> keyNotifier;
    typedef QueuedNotifier<AutomationKeyEvent>::Listener AsyncListener;


    void addAsyncKeyListener(AsyncListener* newListener) { keyNotifier.addListener(newListener); }
    void addAsyncCoalescedKeyListener(AsyncListener* newListener) { keyNotifier.addAsyncCoalescedListener(newListener); }
    void removeAsyncKeyListener(AsyncListener* listener) { keyNotifier.removeListener(listener); }

private:
    juce::WeakReference<AutomationKey>::Master masterReference;
    friend class juce::WeakReference<AutomationKey>;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationKey)
};

