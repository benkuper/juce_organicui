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

	juce::File lastFile;
	juce::String appLayoutExtension = "layout";
	juce::String appSubFolder = "layouts";
	juce::File layoutFolder;

	juce::OwnedArray<ShapeShifterPanel> openedPanels;
	juce::OwnedArray<ShapeShifterWindow> openedWindows;

	ShapeShifterPanel * currentCandidatePanel;

	char * defaultFileData;

	juce::var ghostLayout; //for temporary full panels
	ShapeShifterContent* temporaryFullContent;
	bool saveLastLayout;

	bool lockMode;
	juce::HashMap<juce::String, juce::String> isInViewSubMenu;

	void setDefaultFileData(const char *data);
	void setLayoutInformations(const juce::String &appLayoutExtension, const juce::String &appSubLayoutFolder = "layouts");

	void setCurrentCandidatePanel(ShapeShifterPanel *);

	ShapeShifterPanel * getPanelForContent(ShapeShifterContent * content);
	ShapeShifterPanel * getPanelForContentName(const juce::String & name);

	ShapeShifterPanel * createPanel(ShapeShifterContent * content, ShapeShifterPanelTab * sourceTab = nullptr);
	void removePanel(ShapeShifterPanel * panel);

	ShapeShifterWindow * showPanelWindow(ShapeShifterPanel * _panel, juce::Rectangle<int> bounds);
	ShapeShifterWindow * showPanelWindowForContent(const juce::String &panelName);
	ShapeShifterContent * showContent(juce::String contentName, juce::String attachToContent = juce::String());
	

	void closePanelWindow(ShapeShifterWindow * window, bool doRemovePanel);

	ShapeShifterContent * getContentForName(const juce::String &contentName);

	template<class T>
	T * getContentForType();

	ShapeShifterPanel * checkCandidateTargetForPanel(ShapeShifterPanel * panel);
	bool checkDropOnCandidateTarget(juce::WeakReference<ShapeShifterPanel> panel);

	ShapeShifterWindow * getWindowForPanel(ShapeShifterPanel * panel);

	void loadLayout(juce::var layoutObject);
	juce::var getCurrentLayout();
	void loadLayoutFromFile(int fileIndexInLayoutFolder =-1);
	void loadLayoutFromFile(const juce::File &fromFile);
	void loadLastSessionLayoutFile();
	void loadDefaultLayoutFile(bool forceEmbeddedLayout = false);
	void saveCurrentLayout();
	void saveCurrentLayoutToFile(const juce::File &toFile);
	juce::Array<juce::File> getLayoutFiles();

	void toggleTemporaryFullContent(ShapeShifterContent* content);

	void clearAllPanelsAndWindows();

	const int baseMenuCommandID = 0x31000;
	const int baseSpecialMenuCommandID = 0x32000;
	juce::PopupMenu getPanelsMenu();

	juce::Array<juce::CommandID> getCommandIDs();
	void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result);
	void handleMenuPanelCommand(int commandID);
	bool handleCommandID(int commandID);

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
