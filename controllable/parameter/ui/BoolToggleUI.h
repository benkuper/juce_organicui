/*
  ==============================================================================

    BoolToggleUI.h
    Created: 8 Mar 2016 3:47:01pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class BoolToggleUI :
	public ParameterUI
{
public:
    BoolToggleUI(juce::Array<BoolParameter *> parameters, juce::Image onImage = juce::Image(), juce::Image offImage = juce::Image());
    virtual ~BoolToggleUI();

    juce::Image offImage;
    juce::Image onImage;

    bool usingCustomImages;
    bool momentaryMode;

    void setImages(juce::Image onImage, juce::Image offImage = juce::Image());

    void paint(juce::Graphics &g) override;
    void mouseDownInternal(const juce::MouseEvent &e) override;
    void mouseUpInternal(const juce::MouseEvent &e) override;

    virtual void updateUIParamsInternal() override;

protected:
    void valueChanged(const juce::var & ) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoolToggleUI)
};


class BoolButtonToggleUI :
    public BoolToggleUI
{
public:
    BoolButtonToggleUI(BoolParameter* parameter);
    virtual ~BoolButtonToggleUI();
    

    juce::Rectangle<float> buttonRect;

    void paint(juce::Graphics& g) override;
    bool hitTest(int x, int y) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoolButtonToggleUI)
};
