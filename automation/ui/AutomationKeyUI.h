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

    void paint(juce::Graphics& g) override;

    void inspectableSelectionChanged(Inspectable* i) override;
};

class AutomationKeyUI :
    public ItemMinimalUI<AutomationKey>
{
public:
    AutomationKeyUI(AutomationKey* key);
    ~AutomationKeyUI();

    AutomationKeyHandle handle;

    juce::Rectangle<float> valueBounds;

    std::unique_ptr<EasingUI> easingUI;

    void paint(juce::Graphics& g) override; //avoid painting background
    void resized() override;

    void setShowEasingHandles(bool showFirst, bool showLast);

    void updateEasingUI();

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    bool hitTest(int x, int y) override;

    void setValueBounds(const juce::Rectangle<float> valueBounds);
    juce::Point<int> getUIPosForValuePos(const juce::Point<float>& valuePos) const;

    void controllableFeedbackUpdateInternal(Controllable* c) override;


    class KeyUIListener
    {
    public:
        virtual ~KeyUIListener() {}
        virtual void keyEasingHandleMoved(AutomationKeyUI* key, bool syncOtherHandle, bool isFirstHandle) {}
    };

    juce::ListenerList<KeyUIListener> keyUIListeners;
    void addKeyUIListener(KeyUIListener* newListener) { keyUIListeners.add(newListener); }
    void removeKeyUIListener(KeyUIListener* listener) { keyUIListeners.remove(listener); }


    class KeyEditCalloutComponent :
        public juce::Component
    {
    public:
        KeyEditCalloutComponent(AutomationKey* k);
        ~KeyEditCalloutComponent();

        AutomationKey* k;
        std::unique_ptr<ParameterEditor> positionEditor;
        std::unique_ptr<ParameterEditor> valueEditor;

        void resized() override;
       
    };

};