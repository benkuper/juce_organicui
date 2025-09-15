#pragma once

class ScriptManagerEditor :
	public GenericManagerEditor<Script>
{
public:
	ScriptManagerEditor(ScriptManager* manager, bool isRoot);
	~ScriptManagerEditor();

	BaseItem* addItemFromMenu(bool isFromAddButton) override;
};