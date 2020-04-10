#pragma once

class ScriptManagerEditor :
	public GenericManagerEditor<Script>
{
public:
	ScriptManagerEditor(ScriptManager* manager, bool isRoot);
	~ScriptManagerEditor();

	Script* addItemFromMenu(bool isFromAddButton) override;
};