/*
  ==============================================================================

    ParrotPanel.cpp
    Created: 4 Oct 2021 12:53:40pm
    Author:  bkupe

  ==============================================================================
*/


ParrotManagerUI::ParrotManagerUI() :
    ManagerShapeShifterUI(ParrotManager::getInstance()->niceName, ParrotManager::getInstance())
{
    addExistingItems();
}

ParrotManagerUI::~ParrotManagerUI()
{
}
