
ScriptManagerEditor::ScriptManagerEditor(ScriptManager* manager, bool isRoot) :
	GenericManagerEditor(manager, isRoot)
{

}

ScriptManagerEditor::~ScriptManagerEditor()
{
}

Script* ScriptManagerEditor::addItemFromMenu(bool isFromAddButton)
{
	Script* s = GenericManagerEditor::addItemFromMenu(isFromAddButton);
	s->chooseFileScript();
	return s;
}
