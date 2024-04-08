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
	public juce::ChangeListener,
	public juce::ComponentListener
{

public:
	ColorParameterUI(juce::Array<ColorParameter*> parameters);
	~ColorParameterUI();

	juce::Array<ColorParameter*> colorParams;
	juce::var valueOnEditorOpen;
	ColorParameter * colorParam;
	juce::CallOutBox* colorEditor;

	bool dispatchOnDoubleClick;
	bool dispatchOnSingleClick;

	void paint(juce::Graphics &g) override;
	void resized() override;
	void mouseDownInternal(const juce::MouseEvent &e) override;

	void showEditWindowInternal() override;
	void showEditRangeWindowInternal() override;

	void componentBeingDeleted(juce::Component &) override;

protected:
	// Inherited via ChangeListener
	virtual void changeListenerCallback(juce::ChangeBroadcaster * source) override;
	void valueChanged(const juce::var &) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ColorParameterUI)

};