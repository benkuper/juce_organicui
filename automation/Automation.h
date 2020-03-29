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
    Automation(const String& name = "Curve 2D", AutomationRecorder * recorder = nullptr, bool allowKeysOutside = false);
    ~Automation();

    FloatParameter* position;
    FloatParameter* length;
    FloatParameter* value;

    Point2DParameter* valueRange;
    Point2DParameter* viewValueRange;
    bool allowKeysOutside;

    AutomationRecorder* recorder;

    AutomationKey * addKey(const float& position, const float& value, bool addToUndo = false);
    void addKeys(const Array<AutomationKey *> & keys, bool addToUndo = true, bool removeExistingKeys = true);
    void addFromPointsAndSimplify(const Array<Point<float>>& points, bool addToUndo = true, bool removeExistingKeys = true);

    void addItemInternal(AutomationKey* k, var params) override;
    void addItemsInternal(Array<AutomationKey*>, var params) override;
    void removeItemInternal(AutomationKey* k) override;
    void removeItemsInternal() override;

    void updateNextKeys(int start = 0, int end = -1);
    void computeValue();

    void setLength(float newLength, float stretch = false, float stickToEnd = false);

    Point<float> getPosAndValue();
    juce::Rectangle<float> getBounds();

    void updateRange();


    AutomationKey* getKeyForPosition(float pos); //to make binary search instead
    Array<AutomationKey *> getKeysBetweenPositions(float startPos, float endPos); //to make binary search instead

    float getValueAtNormalizedPosition(float pos);
    float getValueAtPosition(float pos);
    float getNormalizedValueAtPosition(float pos);

    void onContainerParameterChanged(Parameter* p) override;
    void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

    void afterLoadJSONDataInternal() override;

    static int compareKeys(AutomationKey* k1, AutomationKey* k2);

    InspectableEditor* getEditor(bool isRoot) override;
};