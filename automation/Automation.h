/*
  ==============================================================================

    Automation.h
    Created: 11 Dec 2016 1:21:37pm
    Author:  Ben

  ==============================================================================
*/

#pragma once


#pragma once

class Automation :
    public BaseManager<AutomationKey>
{
public:
    Automation(const juce::String& name = "Automation", AutomationRecorder * recorder = nullptr, bool allowKeysOutside = false);
    ~Automation();

    FloatParameter* position;
    FloatParameter* length;
    FloatParameter* value;

    Point2DParameter* valueRange;
    Point2DParameter* viewValueRange;
    bool allowKeysOutside;
    
    float positionUnitSteps;

    AutomationRecorder* recorder;
    enum RangeRemapMode { REMAP_ABSOLUTE, REMAP_PROPORTIONAL };
    EnumParameter * rangeRemapMode;

    //Interactive simplification
    juce::Array<juce::Point<float>> interactiveSourcePoints;
    juce::Array<juce::Point<float>> interactiveSimplifiedPoints;


    AutomationKey* addKey(const float& position, const float& value, bool addToUndo = false);
    void addKeys(const juce::Array<AutomationKey*>& keys, bool addToUndo = true, bool removeExistingKeys = true);
    void insertKeyAt(const float& position, bool addToUndo = true);

    void addFromPointsAndSimplifyBezier(const juce::Array<juce::Point<float>>& sourcePoints, bool addToUndo = true, bool removeExistingKeys = true);

    void addFromPointsAndSimplifyLinear(const juce::Array<juce::Point<float>>& sourcePoints, float tolerance, bool addToUndo = true, bool removeExistingKeys = true);
    juce::Array<juce::Point<float>> getLinearSimplifiedPointsFrom(const juce::Array<juce::Point<float>>& sourcePoints, float tolerance, int start = 0, int end = -1);

    void launchInteractiveSimplification(const juce::Array<juce::Point<float>>& sourcePoints);
    void finishInteractiveSimplification();

    void setUnitSteps(float unitSteps);

    void addItemInternal(AutomationKey* k, juce::var params) override;
    void addItemsInternal(juce::Array<AutomationKey*>, juce::var params) override;
    void removeItemInternal(AutomationKey* k) override;
    void removeItemsInternal(juce::Array<AutomationKey*> keys) override;

    juce::Array<AutomationKey*> addItemsFromClipboard(bool showWarning = false) override;

    juce::Array<juce::UndoableAction*> getMoveKeysBy(float start, float offset);
    juce::Array<juce::UndoableAction*> getRemoveTimespan(float start, float end);

    void updateNextKeys(int start = 0, int end = -1);
    void computeValue();

    void setLength(float newLength, bool stretch = false, bool stickToEnd = false);

    juce::Point<float> getPosAndValue();
    juce::Rectangle<float> getBounds();

    void updateRange();

    AutomationKey* getKeyForPosition(float pos, bool trueIfEqual = true); //to make binary search instead
    AutomationKey* getNextKeyForPosition(float pos, bool trueIfEqual = true); //to make binary search instead
    juce::Array<AutomationKey*> getKeysBetweenPositions(float startPos, float endPos); //to make binary search instead

    float getValueAtNormalizedPosition(float pos);
    float getValueAtPosition(float pos);
    float getNormalizedValueAtPosition(float pos);

    void onContainerParameterChanged(Parameter* p) override;
    void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;
    void onControllableStateChanged(Controllable* c) override;
    void onExternalParameterValueChanged(Parameter* p) override;

    void reorderItems() override;

    void afterLoadJSONDataInternal() override;

    static int compareKeys(AutomationKey* k1, AutomationKey* k2);

    static juce::var setLengthFromScript(const juce::var::NativeFunctionArgs& a);
    static juce::var addKeyFromScript(const juce::var::NativeFunctionArgs& a);
    static juce::var getValueAtPositionFromScript(const juce::var::NativeFunctionArgs& a);
    static juce::var getKeyAtPositionFromScript(const juce::var::NativeFunctionArgs& a);
    static juce::var getKeysBetweenFromScript(const juce::var::NativeFunctionArgs& a);

    DECLARE_ASYNC_EVENT(Automation, Automation, automation, { INTERACTIVE_SIMPLIFICATION_CHANGED })

    InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
};