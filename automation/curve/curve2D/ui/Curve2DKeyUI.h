/*
  ==============================================================================

    Curve2DKeyUI.h
    Created: 21 Mar 2020 4:06:36pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Curve2DKeyHandle :
    public InspectableContentComponent
{
public:
    Curve2DKeyHandle(Curve2DKey* key);
    ~Curve2DKeyHandle();

    Curve2DKey* key;
    
    void paint(juce::Graphics& g) override;

    void inspectableSelectionChanged(Inspectable* i) override;
};

class Curve2DKeyUI :
    public BaseItemMinimalUI<Curve2DKey>
{
public:
    Curve2DKeyUI(Curve2DKey* key);
    ~Curve2DKeyUI();

    Curve2DKeyHandle handle;

    juce::Rectangle<float> valueBounds;

    std::unique_ptr<Easing2DUI> easingUI;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void setShowEasingHandles(bool showFirst, bool showLast);

    void setFocus(float relPos, float range);

    void updateEasingUI();

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    
    bool canStartDrag(const juce::MouseEvent& e) override;
    juce::Point<int> getDragOffset() override;

    bool hitTest(int x, int y) override;

    void setValueBounds(const juce::Rectangle<float> valueBounds);
    juce::Point<int> getUIPosForValuePos(const juce::Point<float>& valuePos) const;

    void controllableFeedbackUpdateInternal(Controllable* c) override;


    class KeyUIListener
    {
    public:
        virtual ~KeyUIListener() {}
        virtual void keyEasingHandleMoved(Curve2DKeyUI * key, bool syncOtherHandle, bool isFirstHandle) {}
    };

    juce::ListenerList<KeyUIListener> keyUIListeners;
    void addKeyUIListener(KeyUIListener* newListener) { keyUIListeners.add(newListener); }
    void removeKeyUIListener(KeyUIListener* listener) { keyUIListeners.remove(listener); }

};