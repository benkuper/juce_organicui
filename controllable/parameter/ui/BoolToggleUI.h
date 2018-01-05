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
	public ParameterUI
{
public:
    BoolToggleUI(Parameter * parameter);
    virtual ~BoolToggleUI();

	Image offImage;
	Image onImage;

	Rectangle<int> drawRect;

    bool invertVisuals;

	void setImages(Image onImage, Image offImage);

    void paint(Graphics &g) override;
    void mouseDownInternal(const MouseEvent &e) override;
    void mouseUpInternal(const MouseEvent &e) override;

	bool hitTest(int x, int y) override;

protected:
    void valueChanged(const var & ) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoolToggleUI)
};


#endif  // BOOLTOGGLEUI_H_INCLUDED
