#pragma once

class KeyMappingsContainerEditor :
	public GenericControllableContainerEditor
{
public:
	KeyMappingsContainerEditor(KeyMappingsContainer * settings, bool isRoot);
	~KeyMappingsContainerEditor();

	KeyMappingEditorComponent keyMappingEditor;
	void resizedInternalContent(juce::Rectangle<int> &r) override;
};