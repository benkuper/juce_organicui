/*
  ==============================================================================

    Curve2D.h
    Created: 21 Mar 2020 4:06:01pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Curve2D :
    public BaseManager<Curve2DKey>
{
public:
    Curve2D(const juce::String &name = "Curve 2D");
    ~Curve2D();

    FloatParameter* position;
    FloatParameter* length;
    juce::Rectangle<float> bounds;

    AutomationRecorder* recorder;

    Point2DParameter* value;

    enum ControlMode { MANUAL, AUTOMATION };
    ControlMode controlMode;

    BoolParameter* keySyncMode; //only for automation mode

    //ui
    bool showUIInEditor;
    FloatParameter* focusRange;

    void setControlMode(ControlMode mode);
    
    void addItemInternal(Curve2DKey* k, juce::var params) override;
    void addItemsInternal(juce::Array<Curve2DKey*> keys, juce::var) override;
    void removeItemInternal(Curve2DKey* k) override;
    void removeItemsInternal(juce::Array<Curve2DKey*> keys) override;

    virtual float addFromPointsAndSimplify(juce::Array<juce::Point<float>> points, bool clearBeforeAdd = false, juce::Array<float> pointTimes = juce::Array<float>());

    void updateCurve(bool relativeAutomationKeySyncMode = true);
    void computeValue();

    Curve2DKey* getKeyForPosition(float pos);
    juce::Point<float> getValueAtNormalizedPosition(float pos);
    juce::Point<float> getValueAtPosition(float pos);


    void onContainerParameterChanged(Parameter* p) override;
    void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;
    
    void afterLoadJSONDataInternal() override;

    InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
};