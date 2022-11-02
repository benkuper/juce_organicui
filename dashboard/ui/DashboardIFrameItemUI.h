/*
  ==============================================================================

    DashboardIFrameItemUI.h
    Created: 5 Dec 2021 4:43:49pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class DashboardIFrameItemUI :
    public DashboardItemUI
{
public:
    DashboardIFrameItemUI(DashboardIFrameItem* item);
    ~DashboardIFrameItemUI();

    DashboardIFrameItem* iFrameItem;

#if JUCE_WINDOWS
    WindowsWebView2WebBrowserComponent web;
#elif !JUCE_LINUX
    WebBrowserComponent web;
#endif

    void paint(Graphics& g) override;
    void resized() override;
    virtual void updateEditModeInternal(bool editMode) override;


    void controllableFeedbackUpdateInternal(Controllable*) override;
};