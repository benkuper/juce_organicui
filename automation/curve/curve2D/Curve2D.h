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
    Curve2D(const String &name = "Curve 2D");
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

    void setControlMode(ControlMode mode);
    
    void addItemInternal(Curve2DKey* k, var params) override;
    void addItemsInternal(Array<Curve2DKey*> keys, var) override;
    void removeItemInternal(Curve2DKey* k) override;
    void removeItemsInternal() override;

    virtual float addFromPointsAndSimplify(Array<Point<float>> points, bool clearBeforeAdd = false, Array<float> pointTimes = Array<float>());

    void updateCurve(bool relativeAutomationKeySyncMode = true);
    void computeValue();

    Curve2DKey* getKeyForPosition(float pos);
    Point<float> getValueAtNormalizedPosition(float pos);
    Point<float> getValueAtPosition(float pos);


    void onContainerParameterChanged(Parameter* p) override;
    void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;
    
    void afterLoadJSONDataInternal() override;

    InspectableEditor* getEditorInternal(bool isRoot, Array<Inspectable*> inspectables = Array<Inspectable*>()) override;
};