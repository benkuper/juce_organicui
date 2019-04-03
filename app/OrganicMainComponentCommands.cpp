/*
 ==============================================================================

 MainComponentMenuBar.cpp
 Created: 25 Mar 2016 6:02:02pm
 Author:  Martin Hermant

 ==============================================================================
 */

ApplicationProperties& getAppProperties();


namespace CommandIDs
{
	static const int open = 0x30000;
	static const int save = 0x30001;
	static const int saveAs = 0x30002;
	static const int newFile = 0x30003;
	static const int openLastDocument = 0x30004;
	static const int checkForUpdates = 0x30005;

	static const int editProjectSettings = 0x50001;
	static const int editGlobalSettings = 0x50002;

	static const int showAbout = 0x60000;
	static const int gotoWebsite = 0x60001;
	static const int gotoForum = 0x60002;

	//undo
	static const int undo = 0x40001;
	static const int redo = 0x40002;
	static const int duplicateItems = 0x40003;
	static const int deleteItems = 0x40004;
	static const int selectAll = 0x40005;


	// range ids
	static const int lastFileStartID = 100; // 100 to 200 max
}

void OrganicMainContentComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) {
	const String category("General");


	switch (commandID)
	{
	case CommandIDs::newFile:
		result.setInfo("New", "Creates a new filter graph file", category, 0);
		result.defaultKeypresses.add(KeyPress('n', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::open:
		result.setInfo("Open...", "Opens a filter graph file", category, 0);
		result.defaultKeypresses.add(KeyPress('o', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::openLastDocument:
		result.setInfo("Open Last Document", "Opens a filter graph file", category, 0);
		result.defaultKeypresses.add(KeyPress('o', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::save:
		result.setInfo("Save", "Saves the current graph to a file", category, 0);
		result.defaultKeypresses.add(KeyPress('s', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::saveAs:
		result.setInfo("Save As...",
			"Saves a copy of the current graph to a file",
			category, 0);
		result.defaultKeypresses.add(KeyPress('s', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::checkForUpdates:
		result.setInfo("Check for updates", "Check if updates are available and download latest software", category, 0);
		break;


	case CommandIDs::editProjectSettings:
		result.setInfo("Project Settings...", "Edit the settings related to this project", category, 0);
		result.defaultKeypresses.add(KeyPress(';', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::editGlobalSettings:
		result.setInfo("Preferences", "Edit the general settings related to this application", category, 0);
		result.defaultKeypresses.add(KeyPress(',', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::showAbout:
		result.setInfo("About...", "", category, 0);
		break;

	case CommandIDs::gotoWebsite:
		result.setInfo("Go to website", "", category, 0);
		break;
	case CommandIDs::gotoForum:
		result.setInfo("Go to forum", "", category, 0);
		break;

	case CommandIDs::undo:
		result.setInfo("Undo " + UndoMaster::getInstance()->getUndoDescription(), "Undo the last action", category, 0);
		result.defaultKeypresses.add(KeyPress('z', ModifierKeys::commandModifier, 0));
		result.setActive(UndoMaster::getInstance()->canUndo());
		break;

	case CommandIDs::redo:
		result.setInfo("Redo " + UndoMaster::getInstance()->getRedoDescription(), "Redo the undone action", category, 0);
		result.defaultKeypresses.add(KeyPress('z', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
		result.defaultKeypresses.add(KeyPress('y', ModifierKeys::commandModifier, 0));
		result.setActive(UndoMaster::getInstance()->canRedo());
		break;

	case StandardApplicationCommandIDs::copy:
	{
		Array<BaseItem *> items = InspectableSelectionManager::activeSelectionManager == nullptr ? Array<BaseItem *>() : InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		String s = items.size() > 0 && items[0] != nullptr ? "Copy " + (items.size() > 1 ? String(items.size()) + " items" : items[0]->niceName) : "Nothing to duplicate";
		result.setInfo(s, "Copy the selected items", category, 0);  
		result.defaultKeypresses.add(KeyPress('c', ModifierKeys::commandModifier, 0));
		result.setActive(items.size() > 0 && items[0] != nullptr);
	}
	break;

	case StandardApplicationCommandIDs::cut:
	{
		Array<BaseItem *> items = InspectableSelectionManager::activeSelectionManager == nullptr ? Array<BaseItem *>() : InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		String s = items.size() > 0 && items[0] != nullptr ? "Cut " + (items.size() > 1 ? String(items.size()) + " items" : items[0]->niceName) : "Nothing to duplicate";
		result.setInfo(s, "Cut the selected items", category, 0);
		result.defaultKeypresses.add(KeyPress('x', ModifierKeys::commandModifier, 0));
		result.setActive(items.size() > 0 && items[0] != nullptr);
	}
	break;

	case StandardApplicationCommandIDs::paste:
	{
		bool canPaste = SystemClipboard::getTextFromClipboard().isNotEmpty();
		result.setInfo(canPaste ? "Paste" : "Nothing to paste", "Paste whatever is in the clipboard", category, 0);
		result.defaultKeypresses.add(KeyPress('v', ModifierKeys::commandModifier, 0));
		result.setActive(canPaste);
	}
	break;

	case CommandIDs::duplicateItems:
	{
		Array<BaseItem *> items = InspectableSelectionManager::activeSelectionManager == nullptr ? Array<BaseItem *>() : InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		String s = items.size() > 0 && items[0] != nullptr ? "Duplicate " + (items.size() > 1?String(items.size())+" items":items[0]->niceName): "Nothing to duplicate";
		result.setInfo(s, "Duplicate the selected items", category, 0);
		result.defaultKeypresses.add(KeyPress('d', ModifierKeys::commandModifier, 0));
		result.setActive(items.size() > 0 && items[0] != nullptr);
	}
	break;

	case CommandIDs::deleteItems:
	{
		Array<BaseItem *> items = InspectableSelectionManager::activeSelectionManager == nullptr ? Array<BaseItem *>() : InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		String s = items.size() > 0  && items[0] != nullptr? "Delete " + (items.size() > 1 ? String(items.size()) + " items" : items[0]->niceName) : "Nothing to delete";
		result.setInfo(s, "Delete the selected items", category, 0);
		result.defaultKeypresses.add(KeyPress(KeyPress::deleteKey), KeyPress(KeyPress::backspaceKey));
		result.setActive(items.size() > 0 && items[0] != nullptr);
	}
	break;

	case CommandIDs::selectAll:
	{
		result.setInfo("Select all", "Select all the containing items or siblings", category, 0);
		result.defaultKeypresses.add(KeyPress('a', ModifierKeys::commandModifier, 0));
		result.setActive(true);
	}
	break;



	default:
		JUCEApplication::getInstance()->getCommandInfo(commandID, result);
		break;
	}
}


void OrganicMainContentComponent::getAllCommands(Array<CommandID>& commands) {
	// this returns the set of all commands that this target can perform..
	const CommandID ids[] = {
	  CommandIDs::newFile,
	  CommandIDs::open,
	  CommandIDs::openLastDocument,
	  CommandIDs::save,
	  CommandIDs::saveAs,
	  CommandIDs::checkForUpdates,
	  StandardApplicationCommandIDs::quit,
	   CommandIDs::selectAll,
	   StandardApplicationCommandIDs::copy,
	  StandardApplicationCommandIDs::cut,
	  StandardApplicationCommandIDs::paste,
	  CommandIDs::duplicateItems,
	  CommandIDs::deleteItems,
	  CommandIDs::editGlobalSettings,
	  CommandIDs::editProjectSettings,
	  CommandIDs::undo,
	  CommandIDs::redo
	};

	commands.addArray(ids, numElementsInArray(ids));
}


PopupMenu OrganicMainContentComponent::getMenuForIndex(int /*topLevelMenuIndex*/, const String& menuName) {
	PopupMenu menu;

	if (menuName == "File")
	{
		// "File" menu
		menu.addCommandItem(&getCommandManager(), CommandIDs::newFile);
		menu.addCommandItem(&getCommandManager(), CommandIDs::open);
		menu.addCommandItem(&getCommandManager(), CommandIDs::openLastDocument);

		RecentlyOpenedFilesList recentFiles;
		recentFiles.restoreFromString(getAppProperties().getUserSettings()
			->getValue("recentFiles"));

		PopupMenu recentFilesMenu;
		recentFiles.createPopupMenuItems(recentFilesMenu, CommandIDs::lastFileStartID, true, true);
		menu.addSubMenu("Open recent file", recentFilesMenu);

		menu.addCommandItem(&getCommandManager(), CommandIDs::save);
		menu.addCommandItem(&getCommandManager(), CommandIDs::saveAs);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::editProjectSettings);
		menu.addCommandItem(&getCommandManager(), CommandIDs::editGlobalSettings);

		fillFileMenuInternal(menu);

		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::checkForUpdates);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::quit);


	} else if (menuName == "Edit")
	{
		menu.addCommandItem(&getCommandManager(), CommandIDs::undo);
		menu.addCommandItem(&getCommandManager(), CommandIDs::redo);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::selectAll);
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::copy);
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::cut);
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::paste);
		menu.addCommandItem(&getCommandManager(), CommandIDs::deleteItems);

	} else if (menuName == "Panels")
	{
		return ShapeShifterManager::getInstance()->getPanelsMenu();

	} 

	return menu;
}

bool OrganicMainContentComponent::perform(const InvocationInfo& info) {

	switch (info.commandID)
	{


	case CommandIDs::newFile:
	{
		FileBasedDocument::SaveResult result = Engine::mainEngine->saveIfNeededAndUserAgrees();
		if (result == FileBasedDocument::SaveResult::userCancelledSave)
		{

		} else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
		{
			LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
		} else
		{
			Engine::mainEngine->createNewGraph();
		}
	}
	break;

	case CommandIDs::open:
	{
		FileBasedDocument::SaveResult result = Engine::mainEngine->saveIfNeededAndUserAgrees();
		if (result == FileBasedDocument::SaveResult::userCancelledSave)
		{

		} else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
		{
			LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
		} else
		{
			Engine::mainEngine->loadFromUserSpecifiedFile(true);
		}
	}
	break;

	case CommandIDs::openLastDocument:
	{
		FileBasedDocument::SaveResult result = Engine::mainEngine->saveIfNeededAndUserAgrees();
		if (result == FileBasedDocument::SaveResult::userCancelledSave)
		{
		} else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
		{
			LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
		} else
		{
			Engine::mainEngine->loadFrom(Engine::mainEngine->getLastDocumentOpened(), true);
		}
	}
	break;

	case CommandIDs::save:
	case CommandIDs::saveAs:
	{
		FileBasedDocument::SaveResult result = FileBasedDocument::SaveResult::savedOk;
		if (info.commandID == CommandIDs::save) result = Engine::mainEngine->save(true, true);
		else result = Engine::mainEngine->saveAs(File(), true, true, true);

		if (result == FileBasedDocument::SaveResult::userCancelledSave)
		{

		} else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
		{
			LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
		} else if (result == FileBasedDocument::SaveResult::savedOk)
		{
			LOG("File saved.");
		}
	}
	break;

	case CommandIDs::checkForUpdates:
		AppUpdater::getInstance()->checkForUpdates();
		break;

	case CommandIDs::undo:
		UndoMaster::getInstance()->undo();
		break;

	case CommandIDs::redo:
		UndoMaster::getInstance()->redo();
		break;


	case StandardApplicationCommandIDs::copy:
	case StandardApplicationCommandIDs::cut:
	{
		Array<BaseItem *> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		if (!items.isEmpty())
		{
			var data = new DynamicObject();
			data.getDynamicObject()->setProperty("itemType", items[0]->itemDataType);

			var itemsData = var();
			for (auto &i : items)
			{
				itemsData.append(i->getJSONData());
				if (info.commandID == StandardApplicationCommandIDs::cut) i->remove();
			}

			data.getDynamicObject()->setProperty("items", itemsData);
			SystemClipboard::copyTextToClipboard(JSON::toString(data));
			LOG(items.size() << "items copied to clipboard");
		}
	}

	break;

	case StandardApplicationCommandIDs::paste:
	{
		if (InspectableSelectionManager::activeSelectionManager != nullptr)
		{
			BaseItem * item = InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>();
			if (item != nullptr) item->paste();
			else
			{
				BaseItem::Listener * m = InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem::Listener>();
				if (m != nullptr) m->askForPaste();
			}
		}
	}
	break;

	case CommandIDs::duplicateItems:
	{
		if (InspectableSelectionManager::activeSelectionManager != nullptr)
		{
			Array<BaseItem *> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
			for(auto &i : items) i->duplicate();
		}
	}
	break;

	case CommandIDs::deleteItems:
	{
		if (InspectableSelectionManager::activeSelectionManager != nullptr)
		{
			Array<WeakReference<Inspectable>> items = InspectableSelectionManager::activeSelectionManager->currentInspectables;
			for (auto &i : items)
			{
				if(!i.wasObjectDeleted()) ((BaseItem *)i.get())->remove();
			}
		}
	}
	break;

	case CommandIDs::selectAll:
	{
		BaseItem * item = InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem>();
		if (item != nullptr) item->selectAll();
		else
		{
			BaseItem::Listener * m = InspectableSelectionManager::activeSelectionManager->getInspectableAs<BaseItem::Listener>();
			if (m != nullptr) m->askForSelectAllItems();
		}
	}
	break;

	case CommandIDs::editProjectSettings:
		ProjectSettings::getInstance()->selectThis();
		break;

	case CommandIDs::editGlobalSettings:
		GlobalSettings::getInstance()->selectThis();
		break;

	default:
		DBG("no command found");
		return JUCEApplication::getInstance()->perform(info);
}

return true;
}

void OrganicMainContentComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	String menuName = getMenuBarNames()[topLevelMenuIndex];

	if (menuName == "Panels")
	{
		ShapeShifterManager::getInstance()->handleMenuPanelCommand(menuItemID);

	} else if (isPositiveAndBelow(menuItemID - CommandIDs::lastFileStartID, 100)) {
		RecentlyOpenedFilesList recentFiles;
		recentFiles.restoreFromString(getAppProperties().getUserSettings()
			->getValue("recentFiles"));
		Engine::mainEngine->loadFrom(recentFiles.getFile(menuItemID - CommandIDs::lastFileStartID), true);
	}
}


StringArray OrganicMainContentComponent::getMenuBarNames() {
	String names[3]{ "File", "Edit", "Panels" };
	return StringArray(names,numElementsInArray(names));
}
