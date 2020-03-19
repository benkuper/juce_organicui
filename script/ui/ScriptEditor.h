/*
  ==============================================================================

    ScriptEditor.h
    Created: 20 Feb 2017 5:04:20pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ScriptEditor :
	public BaseItemEditor,
	public Script::AsyncListener
{
public:
	ScriptEditor(Script * script, bool isRoot); 
	~ScriptEditor();

	Script * script;
	bool editMode;

	std::unique_ptr<ImageButton> fileBT;
	std::unique_ptr<TriggerImageUI> reloadBT;
	std::unique_ptr<ImageButton> editBT;
	std::unique_ptr<BoolToggleUI> logUI;

	std::unique_ptr<InspectableEditor> paramsEditor;

	 juce::Rectangle<int> statusBounds;

	void paint(Graphics &g) override;
	void resizedInternalHeaderItemInternal(juce::Rectangle<int> &r) override;
	void newMessage(const Script::ScriptEvent &e) override;

	void buttonClicked(Button * b) override;
};
