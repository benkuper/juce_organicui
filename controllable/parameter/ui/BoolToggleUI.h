/*
  ==============================================================================

    BoolToggleUI.h
    Created: 8 Mar 2016 3:47:01pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef BOOLTOGGLEUI_H_INCLUDED
#define BOOLTOGGLEUI_H_INCLUDED

class BoolToggleUI :
	public ParameterUI,
    public Timer
{
public:
    BoolToggleUI(Parameter * parameter);
    virtual ~BoolToggleUI();

	Image offImage;
	Image onImage;

    juce::Rectangle<int> drawRect;

    bool invertVisuals;
    
    bool shouldRepaint;

	void setImages(Image onImage, Image offImage);

    void paint(Graphics &g) override;
    void mouseDownInternal(const MouseEvent &e) override;
    void mouseUpInternal(const MouseEvent &e) override;

	bool hitTest(int x, int y) override;
    
    void timerCallback() override;

protected:
    void valueChanged(const var & ) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoolToggleUI)
};


#endif  // BOOLTOGGLEUI_H_INCLUDED
