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
    public Timer
{
public:
    AutomationUI(Automation* manager);
    ~AutomationUI();

    Point<float> viewPosRange;
    float viewLength;
    bool autoAdaptViewRange;

    bool shouldRepaint;

    bool paintingMode;
    Array<Point<float>> paintingPoints;
    Point<float> lastPaintingPoint;

    bool previewMode; //avoid repainting everything
    bool showNumberLines;

    Point<float> viewValueRangeAtMouseDown;

    std::unique_ptr<FloatSliderUI> interactiveSimplificationUI;
    std::unique_ptr<TextButton> validInteractiveBT;

    void paint(Graphics& g) override;
    void drawLinesBackground(Graphics& g);

    void paintOverChildren(Graphics& g) override;

    void resized() override;
    void placeKeyUI(AutomationKeyUI* ui);
    void updateHandlesForUI(AutomationKeyUI* ui, bool checkSideItems);

    void setPreviewMode(bool value);

    void setViewRange(float start, float end);
    void updateItemsVisibility() override;

    void addItemUIInternal(AutomationKeyUI* ui) override;
    void removeItemUIInternal(AutomationKeyUI* ui) override;

    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseDoubleClick(const MouseEvent& e) override;

    void addItemFromMenu(AutomationKey* k, bool fromAddbutton, Point<int> pos) override;

    void addMenuExtraItems(PopupMenu &p, int startIndex) override;
    void handleMenuExtraItemsResult(int result, int startIndex) override;

    Component* getSelectableComponentForItemUI(AutomationKeyUI* ui) override;

    Point<float> getViewPos(Point<int> pos, bool relative = false);
    Rectangle<float> getViewBounds(Rectangle<int> pos, bool relative = false);
    Point<int> getPosInView(Point<float> pos, bool relative = false);
    Rectangle<int> getBoundsInView(Rectangle<float> pos, bool relative = false);

    float getPosForX(int x, bool relative = false);
    int getXForPos(float x, bool relative = false);
    float getValueForY(int y, bool relative = false);
    int getYForValue(float x, bool relative = false);

    void newMessage(const AutomationKey::AutomationKeyEvent& e) override;
    void newMessage(const ContainerAsyncEvent& e) override;
    void newMessage(const Automation::AutomationEvent& e) override;

    void keyEasingHandleMoved(AutomationKeyUI* ui, bool syncOtherHandle, bool isFirst) override;

    void timerCallback() override;

    void buttonClicked(Button* b) override;
};
