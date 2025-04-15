/*
 ==============================================================================

 MainComponentMenuBar.cpp
 Created: 25 Mar 2016 6:02:02pm
 Author:  Martin Hermant

 ==============================================================================
 */

#include "JuceHeader.h"

ApplicationProperties& getAppProperties();
OrganicApplication::MainWindow* getMainWindow();

namespace CommandIDs
{
	static const int open = 0x30000;
	static const int save = 0x30001;
	static const int saveAs = 0x30002;
	static const int saveCopy = 0x30008;
	static const int newFile = 0x30003;
	static const int openLastDocument = 0x30004;
	static const int checkForUpdates = 0x30005;
#if ORGANICUI_USE_WEBSERVER
	static const int updateDashboardFiles = 0x30006;
#endif

	//undo
	static const int undo = 0x40001;
	static const int redo = 0x40002;
	static const int duplicateItems = 0x40003;
	static const int deleteItems = 0x40004;
	static const int selectAll = 0x40005;
	static const int switchDashboardEditMode = 0x40006;

	static const int editProjectSettings = 0x50001;
	static const int editGlobalSettings = 0x50002;
	static const int toggleKioskMode = 0x50003;
	static const int clearGlobalSettings = 0x50004;

	static const int showAbout = 0x60000;

	//navigation
	static const int selectPreviousItem = 0x70001;
	static const int selectNextItem = 0x70002;
	static const int moveToPrevious = 0x70003;
	static const int moveToNext = 0x70004;

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

	case CommandIDs::saveCopy:
		result.setInfo("Save Copy",
			"Saves a copy of the current graph to a file",
			category, 0);

		result.defaultKeypresses.add(KeyPress('s', ModifierKeys::commandModifier | ModifierKeys::altModifier, 0));
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

	case CommandIDs::clearGlobalSettings:
		result.setInfo("Clear Preferences", "If you got some funky things going on, just clear your preferences and hopefully everything will be awesome again !", category, 0);
		break;

	case CommandIDs::toggleKioskMode:
		result.setInfo("Toggle Fullscreen", "Toggle between window mode and kiosk mode", category, 0);
		result.defaultKeypresses.add(KeyPress(KeyPress::F11Key));
		break;

	case CommandIDs::showAbout:
		result.setInfo("About...", "", category, 0);
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
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<ControllableContainer*> items = selectionManager == nullptr ? Array<ControllableContainer*>() : selectionManager->getInspectablesAs<ControllableContainer>();
		String s = items.size() > 0 && items[0] != nullptr ? "Copy " + (items.size() > 1 ? String(items.size()) + " items" : items[0]->niceName) : "Nothing to duplicate";
		result.setInfo(s, "Copy the selected items", category, 0);
		result.defaultKeypresses.add(KeyPress('c', ModifierKeys::commandModifier, 0));
		result.setActive(items.size() > 0 && items[0] != nullptr);
	}
	break;

	case StandardApplicationCommandIDs::cut:
	{
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager == nullptr ? Array<BaseItem*>() : selectionManager->getInspectablesAs<BaseItem>();
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
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager == nullptr ? Array<BaseItem*>() : selectionManager->getInspectablesAs<BaseItem>();
		String s = items.size() > 0 && items[0] != nullptr ? "Duplicate " + (items.size() > 1 ? String(items.size()) + " items" : items[0]->niceName) : "Nothing to duplicate";
		result.setInfo(s, "Duplicate the selected items", category, 0);
		result.defaultKeypresses.add(KeyPress('d', ModifierKeys::commandModifier, 0));
		result.setActive(items.size() > 0 && items[0] != nullptr);
	}
	break;

	case CommandIDs::deleteItems:
	{
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager == nullptr ? Array<BaseItem*>() : selectionManager->getInspectablesAs<BaseItem>();
		String s = items.size() > 0 && items[0] != nullptr ? "Delete " + (items.size() > 1 ? String(items.size()) + " items" : items[0]->niceName) : "Nothing to delete";
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

	case CommandIDs::selectPreviousItem:
	{
		result.setInfo("Select Previous Item", "Select previous item in the list", category, 0);
		result.defaultKeypresses.add(KeyPress(KeyPress::upKey), KeyPress(KeyPress::upKey, ModifierKeys::shiftModifier, 0));

		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager == nullptr ? Array<BaseItem*>() : selectionManager->getInspectablesAs<BaseItem>();


		result.setActive(items.size() > 0);
	}
	break;

	case CommandIDs::selectNextItem:
	{
		result.setInfo("Select Next Item", "Select next item in the list", category, 0);
		result.defaultKeypresses.add(KeyPress(KeyPress::downKey), KeyPress(KeyPress::downKey, ModifierKeys::shiftModifier, 0));

		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager == nullptr ? Array<BaseItem*>() : selectionManager->getInspectablesAs<BaseItem>();

		result.setActive(items.size() > 0);
	}
	break;

	case CommandIDs::moveToPrevious:
	{
		result.setInfo("Move to previous", "Swap this item with the previous one", category, 0);
		result.defaultKeypresses.add(KeyPress(KeyPress::upKey, ModifierKeys::altModifier, 0));

		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager == nullptr ? Array<BaseItem*>() : selectionManager->getInspectablesAs<BaseItem>();


		result.setActive(items.size() > 0);
	}
	break;

	case CommandIDs::moveToNext:
	{
		result.setInfo("Move to next", "Swap this item with the next one", category, 0);
		result.defaultKeypresses.add(KeyPress(KeyPress::downKey, ModifierKeys::altModifier, 0));

		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager == nullptr ? Array<BaseItem*>() : selectionManager->getInspectablesAs<BaseItem>();

		result.setActive(items.size() > 0);
	}
	break;

	case CommandIDs::switchDashboardEditMode:
	{
		result.setInfo("Toggle Dashboard Edit Mode", "Toggle between edit and play mode for the dashboard", category, 0);
		result.defaultKeypresses.add(KeyPress('e', ModifierKeys::commandModifier, 0));
	}
	break;

#if ORGANICUI_USE_WEBSERVER
	case CommandIDs::updateDashboardFiles:
	{
		result.setInfo("Update Dashboard Files", "Update the Dashboard server files", category, 0);
	}
	break;
#endif

	default:
		ShapeShifterManager::getInstance()->getCommandInfo(commandID, result);

#if TIMELINE_ADD_MENU_ITEMS
		TimelineAppCommands::getCommandInfo(commandID, result);
#endif

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
	  CommandIDs::saveCopy,
	  CommandIDs::checkForUpdates,
#if ORGANICUI_USE_WEBSERVER
	  CommandIDs::updateDashboardFiles,
#endif
	  StandardApplicationCommandIDs::quit,
	  CommandIDs::selectAll,
	  StandardApplicationCommandIDs::copy,
	  StandardApplicationCommandIDs::cut,
	  StandardApplicationCommandIDs::paste,
	  CommandIDs::duplicateItems,
	  CommandIDs::deleteItems,
	  CommandIDs::switchDashboardEditMode,
	  CommandIDs::editGlobalSettings,
	  CommandIDs::clearGlobalSettings,
	  CommandIDs::editProjectSettings,
	  CommandIDs::toggleKioskMode,
	  CommandIDs::selectPreviousItem,
	  CommandIDs::selectNextItem,
	  CommandIDs::moveToPrevious,
	  CommandIDs::moveToNext,
	  CommandIDs::undo,
	  CommandIDs::redo
	};

	commands.addArray(ids, numElementsInArray(ids));

	Array<CommandID> shapeShifterCommands = ShapeShifterManager::getInstance()->getCommandIDs();
	commands.addArray(shapeShifterCommands);

#if TIMELINE_ADD_MENU_ITEMS
	TimelineAppCommands::getAllCommands(commands);
#endif
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
		menu.addCommandItem(&getCommandManager(), CommandIDs::saveCopy);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::editProjectSettings);
		menu.addCommandItem(&getCommandManager(), CommandIDs::editGlobalSettings);
		menu.addCommandItem(&getCommandManager(), CommandIDs::clearGlobalSettings);

		fillFileMenuInternal(menu);

		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::checkForUpdates);
#if ORGANICUI_USE_WEBSERVER
		menu.addCommandItem(&getCommandManager(), CommandIDs::updateDashboardFiles);
#endif
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::quit);


	}
	else if (menuName == "Edit")
	{
		menu.addCommandItem(&getCommandManager(), CommandIDs::undo);
		menu.addCommandItem(&getCommandManager(), CommandIDs::redo);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::selectAll);
		menu.addCommandItem(&getCommandManager(), CommandIDs::selectPreviousItem);
		menu.addCommandItem(&getCommandManager(), CommandIDs::selectNextItem);
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::copy);
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::cut);
		menu.addCommandItem(&getCommandManager(), StandardApplicationCommandIDs::paste);
		menu.addCommandItem(&getCommandManager(), CommandIDs::deleteItems);
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::switchDashboardEditMode);

	}
	else if (menuName == "View")
	{
		menu = ShapeShifterManager::getInstance()->getPanelsMenu();
		menu.addSeparator();
		menu.addCommandItem(&getCommandManager(), CommandIDs::toggleKioskMode);
	}

#if TIMELINE_ADD_MENU_ITEMS
	TimelineAppCommands::fillMenu(&getCommandManager(), &menu, menuName);
#endif

	return menu;
}

bool OrganicMainContentComponent::perform(const InvocationInfo& info) {

#if TIMELINE_ADD_MENU_ITEMS
	if (info.commandID >= 0x80000 && info.commandID < 0x81000) return TimelineAppCommands::perform(info);
#endif

	switch (info.commandID)
	{


	case CommandIDs::newFile:
	{
		Engine::mainEngine->saveIfNeededAndUserAgreesAsync([](FileBasedDocument::SaveResult result)
			{
				if (result == FileBasedDocument::SaveResult::userCancelledSave)
				{

				}
				else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
				{
					LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
				}
				else
				{
					Engine::mainEngine->createNewGraph();
				}
			}
		);
	}
	break;

	case CommandIDs::open:
	{
		Engine::mainEngine->saveIfNeededAndUserAgreesAsync([](FileBasedDocument::SaveResult result)
			{
				if (result == FileBasedDocument::SaveResult::userCancelledSave)
				{

				}
				else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
				{
					LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
				}
				else
				{
					Engine::mainEngine->loadFromUserSpecifiedFileAsync(true, [](Result r)
						{
							if (r.failed()) LOGERROR("Load failed :" + r.getErrorMessage());
						}
					);
				}
			}
		);
	}
	break;

	case CommandIDs::openLastDocument:
	{
		Engine::mainEngine->saveIfNeededAndUserAgreesAsync([](FileBasedDocument::SaveResult result)
			{
				if (result == FileBasedDocument::SaveResult::userCancelledSave)
				{
				}
				else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
				{
					LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
				}
				else
				{
					Engine::mainEngine->loadFrom(Engine::mainEngine->getLastDocumentOpened(), true);
				}
			}
		);
	}
	break;

	case CommandIDs::save:
	case CommandIDs::saveAs:
	{

		std::function<void(FileBasedDocument::SaveResult)> rFunc = [](FileBasedDocument::SaveResult result)
			{
				if (result == FileBasedDocument::SaveResult::userCancelledSave)
				{

				}
				else if (result == FileBasedDocument::SaveResult::failedToWriteToFile)
				{
					LOGERROR("Could not save the document (Failed to write to file)\nCancelled loading of the new document");
				}
				else if (result == FileBasedDocument::SaveResult::savedOk)
				{
					LOG("File saved.");
				}
			};

		if (info.commandID == CommandIDs::save) Engine::mainEngine->saveAsync(true, true, rFunc);
		else Engine::mainEngine->saveAsAsync(File(), true, true, true, rFunc);

	}
	break;

	case CommandIDs::saveCopy:
		Engine::mainEngine->saveCopy();
		break;

	case CommandIDs::checkForUpdates:
		AppUpdater::getInstance()->checkForUpdates(true);
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
		Array<BaseItem*> items = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<BaseItem>();
		if (!items.isEmpty())
		{
			var data = new DynamicObject();
			data.getDynamicObject()->setProperty("itemType", items[0]->itemDataType.isNotEmpty() ? items[0]->itemDataType : items[0]->getTypeString());

			var itemsData = var();
			for (auto& i : items)
			{
				if (i == nullptr) continue;
				if (!i->userCanDuplicate) continue;
				itemsData.append(i->getJSONData());
				if (info.commandID == StandardApplicationCommandIDs::cut) i->remove();
			}

			data.getDynamicObject()->setProperty("items", itemsData);
			SystemClipboard::copyTextToClipboard(JSON::toString(data));
			LOG(items.size() << " items copied to clipboard");
			break;
		}

		Array<ControllableContainer*> containers = InspectableSelectionManager::activeSelectionManager->getInspectablesAs<ControllableContainer>();
		if (!containers.isEmpty())
		{

			var ccData = var();
			for (auto& i : containers)
			{
				if (i == nullptr) continue;
				ccData.append(i->getJSONData());
			}

			SystemClipboard::copyTextToClipboard(JSON::toString(ccData.size() > 1 ? ccData : ccData[0]));
			LOG(ccData.size() << " containers copied to clipboard");
			break;
		}
	}

	break;

	case StandardApplicationCommandIDs::paste:
	{
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;

		BaseItem* item = selectionManager->getInspectableAs<BaseItem>();
		if (item != nullptr) item->paste();
		else
		{
			BaseItemListener* m = selectionManager->getInspectableAs<BaseItemListener>();
			if (m != nullptr) m->askForPaste();
		}
	}
	break;

	case CommandIDs::duplicateItems:
	{
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager->getInspectablesAs<BaseItem>();
		HashMap<Manager<BaseItem>*, Array<BaseItem*>*> managerItemMap;

		if (items.size() == 1)
		{
			if (items[0] != nullptr && items[0]->userCanDuplicate) items[0]->duplicate();
		}
		else
		{

			for (auto& i : items)
			{
				if (i == nullptr) continue;
				if (!i->userCanDuplicate) continue;

				if (Manager<BaseItem>* managerContainer = (Manager<BaseItem>*)(i->parentContainer.get()))
				{
					if (!managerItemMap.contains(managerContainer)) managerItemMap.set(managerContainer, new Array<BaseItem*>());
					managerItemMap[managerContainer]->addIfNotAlreadyThere(i);
				}
			}

			HashMap<Manager<BaseItem>*, Array<BaseItem*>*>::Iterator it(managerItemMap);
			while (it.next())
			{
				if (it.getKey() == nullptr)
				{
					for (auto& i : *it.getValue()) i->duplicate();
				}
				else if (it.getValue()->size() == 1)
				{
					it.getValue()->getUnchecked(0)->duplicate();
				}
				else
				{
					var data;
					int maxIndex = 0;
					for (auto& i : *it.getValue())
					{
						var iData = i->getJSONData();
						maxIndex = jmax(it.getKey()->items.indexOf(i), maxIndex);
						data.append(iData);
					}

					maxIndex++;
					for (int i = 0; i < data.size(); i++)
					{
						data[i].getDynamicObject()->setProperty("index", maxIndex++);
					}

					if (data.size() > 0) it.getKey()->addItemsFromData(data);
					delete it.getValue();
				}
			}
		}
	}
	break;

	case CommandIDs::deleteItems:
	{
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager->getInspectablesAs<BaseItem>();
		HashMap<Manager<BaseItem>*, Array<BaseItem*>*> managerItemMap;
		for (auto& i : items)
		{
			if (i == nullptr) continue;
			if (!i->userCanRemove) continue;

			if (Manager<BaseItem>* managerContainer = (Manager<BaseItem>*)(i->parentContainer.get()))
			{
				if (!managerItemMap.contains(managerContainer)) managerItemMap.set(managerContainer, new Array<BaseItem*>());
				managerItemMap[managerContainer]->addIfNotAlreadyThere(i);
			}
		}

		HashMap<Manager<BaseItem>*, Array<BaseItem*>*>::Iterator it(managerItemMap);
		while (it.next())
		{
			if (it.getKey() == nullptr)
			{
				for (auto& i : *it.getValue()) i->remove();
			}
			else
			{
				it.getKey()->removeItems(*it.getValue());
				delete it.getValue();
			}
		}
	}
	break;

	case CommandIDs::selectAll:
	{
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItemListener*> itemListeners = selectionManager->getInspectablesAs<BaseItemListener>();
		if (itemListeners.size() > 0)
		{
			for (auto& m : itemListeners) m->askForSelectAllItems(true);
		}
		else
		{
			Array<BaseItem*> items = selectionManager->getInspectablesAs<BaseItem>();
			for (auto& item : items) item->selectAll(true);
		}
	}
	break;

	case CommandIDs::selectPreviousItem:
	case CommandIDs::selectNextItem:
	case CommandIDs::moveToPrevious:
	case CommandIDs::moveToNext:
	{
		InspectableSelectionManager* selectionManager = InspectableSelectionManager::activeSelectionManager->currentInspectables.size() > 0 ? InspectableSelectionManager::activeSelectionManager : InspectableSelectionManager::mainSelectionManager;
		Array<BaseItem*> items = selectionManager->getInspectablesAs<BaseItem>();
		for (auto& i : items)
		{
			if (i == nullptr) continue;
			if (!i->useCustomArrowKeysBehaviour)
			{
				switch (info.commandID)
				{
				case CommandIDs::selectPreviousItem: i->selectPrevious(info.keyPress.getModifiers().isShiftDown()); break;
				case CommandIDs::selectNextItem: i->selectNext(info.keyPress.getModifiers().isShiftDown()); break;
				case CommandIDs::moveToPrevious: i->moveBefore(); break;
				case CommandIDs::moveToNext: i->moveAfter(); break;
				}
			}
		}
	}
	break;

	case CommandIDs::switchDashboardEditMode:
	{
		if (DashboardManager::getInstanceWithoutCreating() != nullptr) DashboardManager::getInstance()->editMode->setValue(!DashboardManager::getInstance()->editMode->boolValue());
	}
	break;


	case CommandIDs::editProjectSettings:
		ProjectSettings::getInstance()->selectThis();
		break;

	case CommandIDs::editGlobalSettings:
		GlobalSettings::getInstance()->selectThis();
		break;

	case CommandIDs::clearGlobalSettings:
		((OrganicApplication*)OrganicApplication::getInstance())->clearGlobalSettings();
		break;

	case CommandIDs::toggleKioskMode:
		Desktop::getInstance().setKioskModeComponent(Desktop::getInstance().getKioskModeComponent() == getMainWindow() ? nullptr : getMainWindow());
		break;

#if ORGANICUI_USE_WEBSERVER
	case CommandIDs::updateDashboardFiles:
		DashboardManager::getInstance()->downloadDashboardFiles();
		break;
#endif

	default:
		if (ShapeShifterManager::getInstance()->handleCommandID(info.commandID)) return true;
		return JUCEApplication::getInstance()->perform(info);
	}

	return true;
}

void OrganicMainContentComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	String menuName = getMenuBarNames()[topLevelMenuIndex];

	if (menuName == "View" && menuItemID != CommandIDs::toggleKioskMode)
	{
		ShapeShifterManager::getInstance()->handleMenuPanelCommand(menuItemID);

	}
	else if (isPositiveAndBelow(menuItemID - CommandIDs::lastFileStartID, 100)) {
		RecentlyOpenedFilesList recentFiles;
		recentFiles.restoreFromString(getAppProperties().getUserSettings()
			->getValue("recentFiles"));
		Engine::mainEngine->loadFrom(recentFiles.getFile(menuItemID - CommandIDs::lastFileStartID), true);
	}
}


StringArray OrganicMainContentComponent::getMenuBarNames() {
	StringArray names;
	names.addArray({ "File", "Edit" });

#if TIMELINE_ADD_MENU_ITEMS
	names.addArray(TimelineAppCommands::getMenuBarNames());
#endif
	names.add("View");
	return names;
}
