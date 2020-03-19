ApplicationCommandManager& getCommandManager();

KeyMappingsContainerEditor::KeyMappingsContainerEditor(KeyMappingsContainer * km, bool isRoot) :
	GenericControllableContainerEditor(km, isRoot),
	keyMappingEditor(*getCommandManager().getKeyMappings(), true)
{
	
	keyMappingEditor.setColours(Colours::transparentBlack, TEXT_COLOR); 
	addAndMakeVisible(&keyMappingEditor);
	setSize(100, 200);
}

KeyMappingsContainerEditor::~KeyMappingsContainerEditor()
{
}

void KeyMappingsContainerEditor::resizedInternalContent(juce::Rectangle<int>& r)
{
	GenericControllableContainerEditor::resizedInternalContent(r);
	r.translate(0, 8);
	keyMappingEditor.setBounds(r.withHeight(jmax(keyMappingEditor.getHeight(), 200)));
	r.translate(0, keyMappingEditor.getHeight());
}
