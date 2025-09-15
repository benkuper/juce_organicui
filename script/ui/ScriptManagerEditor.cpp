
ScriptManagerEditor::ScriptManagerEditor(ScriptManager* manager, bool isRoot) :
	GenericManagerEditor(manager, isRoot)
{

}

ScriptManagerEditor::~ScriptManagerEditor()
{
}

BaseItem* ScriptManagerEditor::addItemFromMenu(bool isFromAddButton)
{
	BaseItem* s = GenericManagerEditor::addItemFromMenu(isFromAddButton);

	((Script*)s)->chooseFileScript(true);

	return s;
}
