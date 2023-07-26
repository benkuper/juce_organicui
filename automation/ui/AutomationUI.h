/*
  ==============================================================================

    AutomationUI.h
    Created: 11 Dec 2016 1:22:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationUI :
    public BaseManagerUI<Automation, AutomationKey, AutomationKeyUI>,
    public AutomationKey::AsyncListener,
    public AutomationKeyUI::KeyUIListener,
    public ContainerAsyncListener,
    public Automation::AsyncListener,
    public juce::Timer
{
public:
    AutomationUI(Automation* manager);
    ~AutomationUI();

    juce::Point<float> viewPosRange;
    float viewLength;
    bool autoAdaptViewRange;

    bool shouldRepaint;

    bool paintingMode;
    juce::Array<juce::Point<float>> paintingPoints;
    juce::Point<float> lastPaintingPoint;

    bool previewMode; //avoid repainting everything
    bool showNumberLines;
    bool showMenuOnRightClick;

    juce::Point<float> viewValueRangeAtMouseDown;

    std::unique_ptr<FloatSliderUI> interactiveSimplificationUI;
    std::unique_ptr<juce::TextButton> validInteractiveBT;

    juce::Array<float> snapTimes;
    std::function<void(juce::Array<float>*, AutomationKey * k)> getSnapTimesFunc;

    void paint(juce::Graphics& g) override;
    void drawLinesBackground(juce::Graphics& g);

    void paintOverChildren(juce::Graphics& g) override;

    void resized() override;
    void placeKeyUI(AutomationKeyUI* ui);
    void updateHandlesForUI(AutomationKeyUI* ui, bool checkSideItems);

    void setPreviewMode(bool value);

    void setViewRange(float start, float end);
    void updateItemsVisibility() override;

    void addItemUIInternal(AutomationKeyUI* ui) override;
    void removeItemUIInternal(AutomationKeyUI* ui) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    void addItemFromMenu(AutomationKey* k, bool fromAddbutton, juce::Point<int> pos) override;

    void addMenuExtraItems(juce::PopupMenu &p, int startIndex) override;
    void handleMenuExtraItemsResult(int result, int startIndex) override;

    juce::Component* getSelectableComponentForItemUI(AutomationKeyUI* ui) override;

    juce::Point<float> getViewPos(juce::Point<int> pos, bool relative = false);
    juce::Rectangle<float> getViewBounds(juce::Rectangle<int> pos, bool relative = false);
    juce::Point<int> getPosInView(juce::Point<float> pos, bool relative = false);
    juce::Rectangle<int> getBoundsInView(juce::Rectangle<float> pos, bool relative = false);

    float getPosForX(int x, bool relative = false);
    int getXForPos(float x, bool relative = false);
    float getValueForY(int y, bool relative = false);
    int getYForValue(float x, bool relative = false);

    void newMessage(const AutomationKey::AutomationKeyEvent& e) override;
    void newMessage(const ContainerAsyncEvent& e) override;
    void newMessage(const Automation::AutomationEvent& e) override;

    void keyEasingHandleMoved(AutomationKeyUI* ui, bool syncOtherHandle, bool isFirst) override;

    void timerCallback() override;

    void buttonClicked(juce::Button* b) override;
};
