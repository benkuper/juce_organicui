/*
  ==============================================================================

    AutomationKeyUI.h
    Created: 21 Mar 2020 4:06:36pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class AutomationKeyHandle :
    public InspectableContentComponent
{
public:
    AutomationKeyHandle(AutomationKey* key);
    ~AutomationKeyHandle();

    AutomationKey* key;

    void paint(Graphics& g) override;

    void inspectableSelectionChanged(Inspectable* i) override;
};

class AutomationKeyUI :
    public BaseItemMinimalUI<AutomationKey>
{
public:
    AutomationKeyUI(AutomationKey* key);
    ~AutomationKeyUI();

    AutomationKeyHandle handle;

    juce::Rectangle<float> valueBounds;

    std::unique_ptr<EasingUI> easingUI;

    void paint(Graphics& g) override; //avoid painting background
    void resized() override;

    void setShowEasingHandles(bool showFirst, bool showLast);

    void updateEasingUI();

    void mouseDown(const MouseEvent& e) override;
    void mouseDoubleClick(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    bool hitTest(int x, int y) override;

    void setValueBounds(const juce::Rectangle<float> valueBounds);
    Point<int> getUIPosForValuePos(const Point<float>& valuePos) const;

    void controllableFeedbackUpdateInternal(Controllable* c) override;


    class KeyUIListener
    {
    public:
        virtual ~KeyUIListener() {}
        virtual void keyEasingHandleMoved(AutomationKeyUI* key, bool syncOtherHandle, bool isFirstHandle) {}
    };

    ListenerList<KeyUIListener> keyUIListeners;
    void addKeyUIListener(KeyUIListener* newListener) { keyUIListeners.add(newListener); }
    void removeKeyUIListener(KeyUIListener* listener) { keyUIListeners.remove(listener); }

};