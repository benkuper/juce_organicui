/*
  ==============================================================================

    BoolToggleUI.h
    Created: 8 Mar 2016 3:47:01pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BoolToggleUI :
	public ParameterUI,
    public Timer
{
public:
    BoolToggleUI(BoolParameter * parameter, Image onImage = Image(), Image offImage = Image());
    virtual ~BoolToggleUI();

	Image offImage;
	Image onImage;

    bool shouldRepaint;
	
    void setImages(Image onImage, Image offImage = Image());

    void paint(Graphics &g) override;
    void mouseDownInternal(const MouseEvent &e) override;
    void mouseUpInternal(const MouseEvent &e) override;

    void timerCallback() override;

protected:
    void valueChanged(const var & ) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoolToggleUI)
};


class BoolButtonToggleUI :
    public BoolToggleUI
{
public:
    BoolButtonToggleUI(BoolParameter* parameter);
    virtual ~BoolButtonToggleUI();
    
    void paint(Graphics& g) override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoolButtonToggleUI)
};