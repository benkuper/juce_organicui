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
    Automation(const String& name = "Curve 2D");
    ~Automation();

    FloatParameter* position;
    FloatParameter* length;
    FloatParameter* value;

    juce::Rectangle<float> bounds;

    Point2DParameter* valueRange;
    Point2DParameter* viewValueRange;

    bool showUIInEditor;

    AutomationKey * addKey(const float& position, const float& value, bool addToUndo = false);

    void addItemInternal(AutomationKey* k, var params) override;
    void removeItemInternal(AutomationKey* k) override;

    void updateCurve();
    void computeValue();

    void setLength(float newLength, float stretch = false, float stickToEnd = false);

    Point<float> getPosAndValue();

    void updateRange();

    AutomationKey * getKeyForPosition(float pos); //to make binary search instead

    float getValueAtNormalizedPosition(float pos);
    float getValueAtPosition(float pos);
    float getNormalizedValueAtPosition(float pos);

    void onContainerParameterChanged(Parameter* p) override;
    void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;

    void afterLoadJSONDataInternal() override;

    InspectableEditor* getEditor(bool isRoot) override;
};