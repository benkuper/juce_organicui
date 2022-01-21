/*
  ==============================================================================

	ColorParameterUI.h
	Created: 11 Apr 2017 10:42:03am
	Author:  Ben

  ==============================================================================
*/

#pragma once

class ColorParameterUI :
	public ParameterUI,
	public ChangeListener,
	public ComponentListener
{

public:
	ColorParameterUI(Array<ColorParameter*> parameters);
	~ColorParameterUI();

	Array<ColorParameter*> colorParams;
	var valueOnEditorOpen;
	ColorParameter * colorParam;
	CallOutBox* colorEditor;

	bool dispatchOnDoubleClick;
	bool dispatchOnSingleClick;

	void paint(Graphics &g) override;
	void resized() override;
	void mouseDownInternal(const MouseEvent &e) override;

	void showEditWindowInternal() override;

	void componentBeingDeleted(Component &) override;

protected:
	// Inherited via ChangeListener
	virtual void changeListenerCallback(ChangeBroadcaster * source) override;
	void valueChanged(const var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorParameterUI)

};