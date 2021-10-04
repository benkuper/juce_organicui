/*
  ==============================================================================

    ShapeShifterManager.h
    Created: 2 May 2016 3:11:35pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "ShapeShifterContainer.h"
#include "ShapeShifterWindow.h"
#include "ShapeShifterFactory.h"

class ShapeShifterManager :
	public ShapeShifterPanel::Listener,
	public Parameter::AsyncListener
{
public:
	juce_DeclareSingleton(ShapeShifterManager,true);
	ShapeShifterManager();
	virtual ~ShapeShifterManager();

	ShapeShifterContainer mainContainer;

	File lastFile;
	String appLayoutExtension = "layout";
	String appSubFolder = "layouts";
	File layoutFolder;

	OwnedArray<ShapeShifterPanel> openedPanels;
	OwnedArray<ShapeShifterWindow> openedWindows;

	ShapeShifterPanel * currentCandidatePanel;

	char * defaultFileData;

	var ghostLayout; //for temporary full panels
	ShapeShifterContent* temporaryFullContent;

	bool lockMode;


	void setDefaultFileData(const char *data);
	void setLayoutInformations(const String &appLayoutExtension, const String &appSubLayoutFolder = "layouts");

	void setCurrentCandidatePanel(ShapeShifterPanel *);

	ShapeShifterPanel * getPanelForContent(ShapeShifterContent * content);
	ShapeShifterPanel * getPanelForContentName(const String & name);

	ShapeShifterPanel * createPanel(ShapeShifterContent * content, ShapeShifterPanelTab * sourceTab = nullptr);
	void removePanel(ShapeShifterPanel * panel);

	ShapeShifterWindow * showPanelWindow(ShapeShifterPanel * _panel, juce::Rectangle<int> bounds);
	ShapeShifterWindow * showPanelWindowForContent(const String &panelName);
	ShapeShifterContent * showContent(String contentName);
	

	void closePanelWindow(ShapeShifterWindow * window, bool doRemovePanel);

	ShapeShifterContent * getContentForName(const String &contentName);

	template<class T>
	T * getContentForType();

	ShapeShifterPanel * checkCandidateTargetForPanel(ShapeShifterPanel * panel);
	bool checkDropOnCandidateTarget(WeakReference<ShapeShifterPanel> panel);

	ShapeShifterWindow * getWindowForPanel(ShapeShifterPanel * panel);

	void loadLayout(var layoutObject);
	var getCurrentLayout();
	void loadLayoutFromFile(int fileIndexInLayoutFolder =-1);
	void loadLayoutFromFile(const File &fromFile);
	void loadLastSessionLayoutFile();
	void loadDefaultLayoutFile(bool forceEmbeddedLayout = false);
	void saveCurrentLayout();
	void saveCurrentLayoutToFile(const File &toFile);
	Array<File> getLayoutFiles();

	void toggleTemporaryFullContent(ShapeShifterContent* content);

	void clearAllPanelsAndWindows();

	const int baseMenuCommandID = 0x31000;
	const int baseSpecialMenuCommandID = 0x32000;
	PopupMenu getPanelsMenu();

	void handleMenuPanelCommand(int commandID);

	void newMessage(const Parameter::ParameterEvent& e) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterManager)
};

template<class T>
T * ShapeShifterManager::getContentForType()
{
	for (auto &p : openedPanels)
	{
		for (auto & c : p->contents)
		{
			T * d = dynamic_cast<T *>(c);
			if (d != nullptr) return d;
		}
	}

	return nullptr;
}
