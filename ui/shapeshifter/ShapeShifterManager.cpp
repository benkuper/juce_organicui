/*
  ==============================================================================

	ShapeShifterManager.cpp
	Created: 2 May 2016 3:11:35pm
	Author:  bkupe

  ==============================================================================
 */

#include "JuceHeader.h"

juce_ImplementSingleton(ShapeShifterManager);

static OrganicApplication& getApp();
ApplicationCommandManager& getCommandManager();// { return getApp().commandManager; }


ShapeShifterManager::ShapeShifterManager() :
	mainContainer(ShapeShifterContainer::Direction::VERTICAL),
	currentCandidatePanel(nullptr),
	defaultFileData(nullptr),
	temporaryFullContent(nullptr),
	lockMode(false)
{
	GlobalSettings::getInstance()->fontSize->addAsyncParameterListener(this);
}

ShapeShifterManager::~ShapeShifterManager()
{
	if(layoutFolder.exists()) saveCurrentLayoutToFile(layoutFolder.getChildFile(+"_lastSession." + appLayoutExtension));
	openedWindows.clear();
	if (GlobalSettings::getInstanceWithoutCreating() != nullptr) GlobalSettings::getInstance()->fontSize->removeAsyncParameterListener(this);
}

void ShapeShifterManager::setDefaultFileData(const char* data)
{
	defaultFileData = (char*)data;
}

void ShapeShifterManager::setLayoutInformations(const String& _appLayoutExtension, const String& appSubLayoutFolder)
{
	appLayoutExtension = _appLayoutExtension;
	appSubFolder = appSubLayoutFolder;

	layoutFolder = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getChildFile(appSubFolder);
	if (!layoutFolder.exists())
	{
		Result r = layoutFolder.createDirectory();
		if (r.failed()) LOGERROR("Could not create layout folder in " << layoutFolder.getFullPathName() << ", is it accessible ?");
	}
}

void ShapeShifterManager::setCurrentCandidatePanel(ShapeShifterPanel* panel)
{
	if (currentCandidatePanel == panel) return;

	if (currentCandidatePanel != nullptr) currentCandidatePanel->setTargetMode(false);
	currentCandidatePanel = panel;
	if (currentCandidatePanel != nullptr) currentCandidatePanel->setTargetMode(true);

}

ShapeShifterPanel* ShapeShifterManager::getPanelForContent(ShapeShifterContent* content)
{
	for (auto& p : openedPanels)
	{
		if (p->hasContent(content)) return p;
	}

	return nullptr;
}

ShapeShifterPanel* ShapeShifterManager::getPanelForContentName(const String& name)
{
	for (auto& p : openedPanels)
	{
		if (p->hasContent(name)) return p;
	}

	return nullptr;
}

ShapeShifterPanel* ShapeShifterManager::createPanel(ShapeShifterContent* content, ShapeShifterPanelTab* sourceTab)
{
	if (!getApp().useWindow) return nullptr;

	ShapeShifterPanel* panel = new ShapeShifterPanel(content, sourceTab);

	//if(content != nullptr) panel->setSize(content->getWidth(), content->getHeight());

	//DBG("Add shape shifter panel listener from manager");
	panel->addShapeShifterPanelListener(this);
	openedPanels.add(panel);
	return panel;
}

void ShapeShifterManager::removePanel(ShapeShifterPanel* panel)
{
	panel->removeShapeShifterPanelListener(this);
	openedPanels.removeObject(panel, true);
}

ShapeShifterWindow* ShapeShifterManager::showPanelWindow(ShapeShifterPanel* _panel, juce::Rectangle<int> bounds)
{
	if (!getApp().useWindow) return nullptr;

	ShapeShifterWindow* w = new ShapeShifterWindow(_panel, bounds);
	openedWindows.add(w);
	w->toFront(true);
	return w;
}

ShapeShifterWindow* ShapeShifterManager::showPanelWindowForContent(const String& panelName)
{
	if (!getApp().useWindow) return nullptr;

	ShapeShifterContent* c = ShapeShifterFactory::createContent(panelName);

	if (c == nullptr) return nullptr;

	ShapeShifterPanel* newP = createPanel(c);
	juce::Rectangle<int> r(100, 100, 300, 500);
	ShapeShifterWindow* w = showPanelWindow(newP, r);
	return w;
}

ShapeShifterContent* ShapeShifterManager::showContent(String contentName)
{
	if (!getApp().useWindow)  return nullptr;

	//DBG("Show content " << contentName);
	ShapeShifterPanel* p = getPanelForContentName(contentName);

	if (p != nullptr)
	{
		p->setCurrentContent(contentName);
		ShapeShifterWindow* w = getWindowForPanel(p);
		if (w != nullptr) w->toFront(true);

		return p->currentContent;
	}
	else
	{
		ShapeShifterContent* c = ShapeShifterFactory::createContent(contentName);

		if (c == nullptr) return nullptr;

		ShapeShifterPanel* newP = createPanel(c);

		if (mainContainer.shifters.size() == 0)
		{
			mainContainer.insertPanelAt(newP, 0);
		}
		else
		{
			juce::Rectangle<int> r(100, 100, 300, 500);
			showPanelWindow(newP, r);
		}

		return c;
	}

	return nullptr;
}

void ShapeShifterManager::closePanelWindow(ShapeShifterWindow* window, bool doRemovePanel)
{
	if (window == nullptr) return;
	ShapeShifterPanel* p = window->panel;
	window->clear();
	window->removeFromDesktop();
	if (doRemovePanel) removePanel(p);
	openedWindows.removeObject(window, true);

}

ShapeShifterContent* ShapeShifterManager::getContentForName(const String& contentName)
{
	for (auto& p : openedPanels)
	{
		if (p->hasContent(contentName)) return p->getContentForName(contentName);
	}

	return nullptr;
}

ShapeShifterPanel* ShapeShifterManager::checkCandidateTargetForPanel(ShapeShifterPanel* panel)
{
	ShapeShifterPanel* candidate = nullptr;

	for (auto& p : openedPanels)
	{
		if (p == panel) continue;

		if (p->getLocalBounds().contains(p->getLocalPoint(panel, Point<float>()).toInt()))
		{
			candidate = p;
		}
	}

	setCurrentCandidatePanel(candidate);

	if (currentCandidatePanel != nullptr) currentCandidatePanel->checkAttachZone(panel);


	return candidate;
}

bool ShapeShifterManager::checkDropOnCandidateTarget(WeakReference<ShapeShifterPanel> panel)
{
	if (panel.wasObjectDeleted()) return false;

	if (currentCandidatePanel == nullptr) return false;

	bool result = currentCandidatePanel->attachPanel(panel);
	//if (result) closePanelWindow(getWindowForPanel(panel),false);
	setCurrentCandidatePanel(nullptr);
	return result;
}

ShapeShifterWindow* ShapeShifterManager::getWindowForPanel(ShapeShifterPanel* panel)
{
	for (auto& w : openedWindows)
	{
		if (w->panel == panel) return w;
	}

	return nullptr;
}

void ShapeShifterManager::loadLayout(var layout)
{
	if (!layout.isObject()) return;
	if (!getApp().useWindow) return;

	clearAllPanelsAndWindows();
	mainContainer.loadLayout(layout.getDynamicObject()->getProperty("mainLayout"));

	Array<var>* wData = layout.getDynamicObject()->getProperty("windows").getArray();

	if (wData != nullptr)
	{
		for (auto& wd : *wData)
		{
			DynamicObject* d = wd.getDynamicObject();
			ShapeShifterPanel* p = createPanel(nullptr);
			p->loadLayout(d->getProperty("panel"));
			juce::Rectangle<int> bounds(d->getProperty("x"), d->getProperty("y"), d->getProperty("width"), d->getProperty("height"));
			showPanelWindow(p, bounds);
		}
	}

	if (openedPanels.size() > 0)
	{
		if (openedPanels[0]->isShowing()) openedPanels[0]->grabKeyboardFocus();
	}
}

var ShapeShifterManager::getCurrentLayout()
{
	var layout(new DynamicObject());
	layout.getDynamicObject()->setProperty("mainLayout", mainContainer.getCurrentLayout());

	var wData;
	for (auto& w : openedWindows)
	{
		wData.append(w->getCurrentLayout());
	}

	layout.getDynamicObject()->setProperty("windows", wData);
	return layout;
}

void ShapeShifterManager::loadLayoutFromFile(int fileIndexInLayoutFolder)
{

	if (!layoutFolder.exists()) layoutFolder.createDirectory();

	File layoutFile;
	if (fileIndexInLayoutFolder == -1)
	{
		FileChooser* fc(new FileChooser("Load layout", layoutFolder, "*." + appLayoutExtension));
		auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;
		fc->launchAsync(folderChooserFlags, [this](const FileChooser& fc)
			{
				File f = fc.getResult();
				delete& fc;
				if (f == File()) return;
				loadLayoutFromFile(f);
			}
		);
	}
	else
	{
		Array<File> layoutFiles = getLayoutFiles();
		layoutFile = layoutFiles[fileIndexInLayoutFolder];
		loadLayoutFromFile(layoutFile);
	}

}

void ShapeShifterManager::loadLayoutFromFile(const File& fromFile)
{
	if (!fromFile.existsAsFile()) return;

	std::unique_ptr<InputStream> is(fromFile.createInputStream());
	if (is == nullptr)
	{
		LOGERROR("Error loading layout");
		return;
	}

	var data = JSON::parse(*is);
	loadLayout(data);


}

void ShapeShifterManager::loadLastSessionLayoutFile()
{
	lastFile = layoutFolder.getChildFile("_lastSession." + appLayoutExtension);
	if (lastFile.exists()) loadLayoutFromFile(lastFile);
	else loadDefaultLayoutFile();
}

void ShapeShifterManager::loadDefaultLayoutFile(bool forceEmbeddedLayout)
{
	File defaultFile = layoutFolder.getChildFile("default." + appLayoutExtension);
	if (defaultFile.exists() && !forceEmbeddedLayout)
	{
		loadLayoutFromFile(defaultFile);
	}
	else
	{
		if (defaultFileData != nullptr)
		{
			String defaultLayoutFileData = String::fromUTF8(defaultFileData);
			loadLayout(JSON::parse(defaultLayoutFileData));
			saveCurrentLayoutToFile(defaultFile);
		}

	}
}

void ShapeShifterManager::saveCurrentLayout()
{
	if (!layoutFolder.exists()) layoutFolder.createDirectory();

	FileChooser* fc(new FileChooser("Save layout", layoutFolder, "*." + appLayoutExtension));
	fc->launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::FileChooserFlags::warnAboutOverwriting, [this](const FileChooser& fc)
		{
			File f = fc.getResult();
			delete& fc;
			if (f == File()) return;

			saveCurrentLayoutToFile(f.withFileExtension(appLayoutExtension));
		}
	);
}

void ShapeShifterManager::saveCurrentLayoutToFile(const File& toFile)
{
	if (!layoutFolder.exists()) layoutFolder.createDirectory();

	if (toFile.exists())
	{
		toFile.deleteFile();
		toFile.create();
	}

	var data = temporaryFullContent == nullptr ? getCurrentLayout() : ghostLayout;
	if (data.isUndefined() || data.isVoid()) return;

	DBG("Save layout to file : " << toFile.getFullPathName());
	if (toFile.existsAsFile()) toFile.deleteFile();
	std::unique_ptr<OutputStream> os(toFile.createOutputStream());
	if (os == nullptr)
	{
		NLOGERROR("Shape Shifter", "Error saving the layout file " + toFile.getFullPathName() + "\nMaybe it is read-only ?");
		return;
	}
	JSON::writeToStream(*os, data);
	os->flush();
}

Array<File> ShapeShifterManager::getLayoutFiles()
{
	if (!layoutFolder.exists()) return Array<File>();
	Array<File> layoutFiles;
	layoutFolder.findChildFiles(layoutFiles, File::findFiles, false, "*." + appLayoutExtension);

	return layoutFiles;
}

void ShapeShifterManager::toggleTemporaryFullContent(ShapeShifterContent* content)
{
	if (temporaryFullContent != nullptr) content = nullptr;

	if (content == nullptr)
	{
		loadLayout(ghostLayout);
		temporaryFullContent = nullptr;
	}
	else
	{
		ghostLayout = getCurrentLayout();
		String contentName = content->contentName;

		StringArray otherContents;

		HashMap<ShapeShifterPanelTab*, ShapeShifterPanel*> panelTabRemoveMap;

		for (auto& p : openedPanels)
		{
			for (auto& t : p->header.tabs)
			{
				if (t->content->contentName != contentName && t->content->contentName != "Inspector") panelTabRemoveMap.set(t, p);
			}
		}

		HashMap<ShapeShifterPanelTab*, ShapeShifterPanel*>::Iterator i(panelTabRemoveMap);
		while (i.next()) i.getValue()->removeTab(i.getKey());

		temporaryFullContent = content;

	}
}

void ShapeShifterManager::clearAllPanelsAndWindows()
{
	while (openedWindows.size() > 0)
	{
		closePanelWindow(openedWindows[0], true);
	}
	while (openedPanels.size() > 0)
	{
		removePanel(openedPanels[0]);
	}
	mainContainer.clear();
}

PopupMenu ShapeShifterManager::getPanelsMenu()
{
	PopupMenu p;
	PopupMenu layoutP;
	layoutP.addItem(baseSpecialMenuCommandID + 1, "Save Current layout");
	layoutP.addItem(baseSpecialMenuCommandID + 2, "Load Default Layout");
	layoutP.addItem(baseSpecialMenuCommandID + 3, "Load Layout...");
	layoutP.addSeparator();

	Array<File> layoutFiles = getLayoutFiles();

	//int specialIndex = layoutP.getNumItems() + 2; //+2 to have lockPanels
	int li = 0;
	for (auto& f : layoutFiles)
	{
		layoutP.addCommandItem(&getCommandManager(), 0x9000 + li, f.getFileNameWithoutExtension());
		li++;
	}

	p.addSubMenu("Layout", layoutP);
	p.addItem(baseSpecialMenuCommandID + 4, "Lock Panels", true, lockMode);

	p.addSeparator();

	int currentID = 1;
	HashMap<String, PopupMenu> subMenus;
	Array<String> subMenuNames;
	for (auto& n : ShapeShifterFactory::getInstance()->defs)
	{
		if (isInViewSubMenu.contains(n->contentName)) {
			String subMenuName = isInViewSubMenu.getReference(n->contentName);
			if (!subMenus.contains(subMenuName)) {
				PopupMenu temp;
				subMenus.set(subMenuName, temp);
				subMenuNames.add(subMenuName);

			}
			subMenus.getReference(subMenuName).addItem(baseMenuCommandID + currentID, n->contentName, true);
		}
		else 
		{
			p.addItem(baseMenuCommandID + currentID, n->contentName, true);
		}
		currentID++;
	}

	for (int i = 0; i < subMenuNames.size(); i++) {
		if (subMenuNames[i] != "") {
			p.addSubMenu(subMenuNames[i], subMenus.getReference(subMenuNames[i]));
		}
	}
	return p;
}

Array<CommandID> ShapeShifterManager::getCommandIDs()
{
	Array<CommandID> result;
	for (int i = 0; i < 10; i++) result.add(0x9000 + i); //10 layouts load
	return result;
}

void ShapeShifterManager::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{

	const int keysArr[10]{ 38, 233, 34, 39, 40, 45, 232, 95, 231, 224 }; //for azerty keyboards

	if (commandID >= 0x9000 && commandID < 0x9000 + 10)
	{
		int cid = commandID - 0x9000;
		Array<File> layouts = getLayoutFiles();
		//if (cid >= layouts.size()) return;
		String fName = layouts.size() > cid ? layouts[cid].getFileNameWithoutExtension() : "empty";

		result.setInfo("Load Layout " + String(cid + 1) + " (" + fName + ")", "", "View", 0);
		result.addDefaultKeypress(KeyPress::createFromDescription(String(cid)).getKeyCode(), ModifierKeys::commandModifier);
		result.addDefaultKeypress(keysArr[cid], ModifierKeys::commandModifier | ModifierKeys::shiftModifier);

	}
}

void ShapeShifterManager::handleMenuPanelCommand(int commandID)
{
	//DBG("Handle command " << commandID);



	bool isSpecial = ((commandID & 0xff000) == 0x32000);

	if (isSpecial)
	{
		int sCommandID = commandID - baseSpecialMenuCommandID;
		switch (sCommandID)
		{
		case 1: //Save
			saveCurrentLayout();
			break;

		case 2: //Load
			loadDefaultLayoutFile();
			break;

		case 3: //Load
			loadLayoutFromFile();
			break;

		case 4:
			lockMode = !lockMode;
			LOG("Panels are now " << (lockMode ? "locked" : "unlocked") << ".");
			break;

		default:
			//loadLayoutFromFile(sCommandID - 5);
			break;
		}
		return;
	}

	int relCommandID = commandID - baseMenuCommandID - 1;

	if (relCommandID >= 0 && relCommandID < ShapeShifterFactory::getInstance()->defs.size())
	{

		String contentName = ShapeShifterFactory::getInstance()->defs[relCommandID]->contentName;
		showContent(contentName);
	}
}

bool ShapeShifterManager::handleCommandID(int commandID)
{
	if (commandID >= 0x9000 && commandID < 0x9000 + 10)
	{
		int cid = commandID - 0x9000;
		Array<File> layouts = getLayoutFiles();
		//if (cid >= layouts.size()) return;
		if (layouts.size() > cid) loadLayoutFromFile(layouts[cid]);
		return true;
	}

	return false;
}

void ShapeShifterManager::newMessage(const Parameter::ParameterEvent& e)
{
	if (e.type == e.VALUE_CHANGED && e.parameter == GlobalSettings::getInstance()->fontSize)
	{
		if (Engine::mainEngine->isLoadingFile || Engine::mainEngine->isClearing) return;
		for (auto& p : openedPanels)
		{
			for (auto& t : p->header.tabs)
			{
				t->panelLabel.setFont(GlobalSettings::getInstance()->fontSize->floatValue());
			}
			p->header.resized();
		}

	}
}
