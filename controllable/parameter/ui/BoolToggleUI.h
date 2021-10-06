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
    bool usingCustomImages;
	
    void setImages(Image onImage, Image offImage = Image());

    void paint(Graphics &g) override;
    void mouseDownInternal(const MouseEvent &e) override;
    void mouseUpInternal(const MouseEvent &e) override;

    virtual void updateUIParamsInternal();

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
    

    Rectangle<float> buttonRect;

    void paint(Graphics& g) override;
    bool hitTest(int x, int y) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoolButtonToggleUI)
};