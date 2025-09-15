/*
  ==============================================================================

    Curve2DUI.h
    Created: 21 Mar 2020 4:06:30pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Curve2DUI :
    public ManagerViewUI<Curve2D, Curve2DKey>,
    public Curve2DKey::AsyncListener,
    public Curve2DKeyUI::KeyUIListener,
    public ContainerAsyncListener,
    public AutomationRecorder::AsyncListener
{
public:
    Curve2DUI(Curve2D * manager);
    ~Curve2DUI();

    bool paintingMode;
    juce::Array<juce::Point<float>> paintingPoints;

    void paintOverChildren(juce::Graphics& g) override;

    void updateViewUIPosition(BaseItemMinimalUI * ui) override;
    void updateHandlesForUI(Curve2DKeyUI* ui, bool checkSideItems);

    bool checkItemShouldBeVisible(BaseItemMinimalUI* ui) override;

    void addItemUIInternal(BaseItemMinimalUI* ui) override;
    void removeItemUIInternal(BaseItemMinimalUI* ui) override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void mouseDoubleClick(const juce::MouseEvent& e) override;

    juce::Component* getSelectableComponentForItemUI(BaseItemMinimalUI* ui) override;

    void newMessage(const Curve2DKey::Curve2DKeyEvent& e) override;
    void newMessage(const ContainerAsyncEvent& e) override;
    void newMessage(const AutomationRecorder::RecorderEvent& e) override;

    void keyEasingHandleMoved(Curve2DKeyUI* ui, bool syncOtherHandle, bool isFirst) override;
};