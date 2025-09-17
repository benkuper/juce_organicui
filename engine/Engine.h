/*
 ==============================================================================

 Engine.h
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */
#pragma once

 //#define MULTITHREADED_LOADING

#pragma warning (disable : 4100)


class Engine :
	public ControllableContainer,
	public juce::FileBasedDocument,
	public juce::AsyncUpdater,
	public ProgressNotifier,
	public DashboardItemProvider,
	public juce::MultiTimer //for auto save and CPU
{
public:
	Engine(const juce::String& fileName = "File", const juce::String& fileExtension = ".file");
	virtual ~Engine();

	static Engine* mainEngine;

	std::unique_ptr<InspectableSelectionManager> selectionManager;


	bool isBetaVersion;
	int betaVersion;

	juce::StringArray breakingChangesVersions;
	juce::String convertURL = ""; //for file conversion

	juce::String fileName = "File";
	juce::String fileExtension = ".file";
	juce::String fileWildcard = "*" + fileExtension;

	juce::String lastFileAbsolutePath; //Used for checking in saveDocument if new file is different

	int autoSaveIndex;
	juce::Time lastChangeTime;
	bool isThereChangeToBackup = false; // true if there is real change to backup ( set to true on changed() methods )

	virtual void changed() override;
	void createNewGraph();
	virtual void createNewGraphInternal() {}
	virtual void afterLoadFileInternal() {}

	void clear() override;
	virtual void clearInternal() {}; //to override to clear specific application classes

	virtual bool parseCommandline(const juce::String&);

	//==============================================================================
	// see EngineFileDocument.cpp

	//  inherited from FileBasedDocument
	juce::String getDocumentTitle()override;
	juce::Result loadDocument(const juce::File& file) override;
	juce::Result loadDocumentNoCheck(const juce::File& file);
	juce::Result saveDocument(const juce::File& file) override;
	juce::Result saveCopy();
	juce::Result saveBackupDocument(int index);

	void loadDocumentFromJSON(juce::var data);

	juce::File getLastDocumentOpened() override;
	void setLastDocumentOpened(const juce::File& file) override;

	virtual bool checkAutoRestoreAutosave(const juce::File& originalFile, std::function<void(const juce::File&)> cancelCallback);
	void restoreAutosave(const juce::File& originalFile, const juce::File& autosaveFile);

	//    #if JUCE_MODAL_LOOPS_PERMITTED
	//     File getSuggestedSaveAsFile (const File& defaultFile)override;
	//    #endif

	// our Saving methods
	juce::var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONData(juce::var data, ProgressTask* loadingTask);
	void loadJSONDataEngine(juce::var data, ProgressTask* loadingTask);
	virtual void loadJSONDataInternalEngine(juce::var data, ProgressTask* loadingTask) {}

	bool checkFileVersion(juce::DynamicObject* metaData, bool checkForNewerVersion = false);
	bool versionIsNewerThan(juce::String versionToCheck, juce::String referenceVersion);
	int getBetaVersion(juce::String version);
	bool versionNeedsOnlineUpdate(juce::String version);
	virtual juce::String getMinimumRequiredFileVersion();

	juce::int64 loadingStartTime;

	void fileLoaderEnded();
	bool allLoadingThreadsAreEnded();
	void loadDocumentAsync(const juce::File& file);

	virtual void timerCallback(int timerID) override;


	class FileLoader : public juce::Thread, public juce::Timer {
	public:
		FileLoader(Engine* e, juce::File f) :juce::Thread("EngineLoader"), owner(e), fileToLoad(f) {
			//startTimerHz(4);
			//fakeProgress = 0;
			isEnded = false;
		}
		~FileLoader() {

		}

		void timerCallback()override {
			//fakeProgress+=getTimerInterval()/5000.0;
			//fakeProgress = jmin(1.0f,fakeProgress);
			//owner->engineListeners.call(&EngineListener::fileProgress,fakeProgress, 0);
		}

		void run() override {
			owner->loadDocumentAsync(fileToLoad);
			isEnded = true;
			owner->fileLoaderEnded();
		}

		//float fakeProgress ;
		Engine* owner;
		juce::File fileToLoad;
		bool isEnded;


	};

	std::unique_ptr<FileLoader> fileLoader;

	DECLARE_INSPECTACLE_SAFE_LISTENER(Engine, engine);
	DECLARE_ASYNC_EVENT(Engine, Engine, engine, ENUM_LIST(START_LOAD_FILE, FILE_PROGRESS, END_LOAD_FILE, FILE_LOADED, FILE_SAVED, ENGINE_CLEARED, FILE_CHANGED), EVENT_INSPECTABLE_CHECK);

	bool isLoadingFile;
	bool isClearing;
	juce::var jsonData;

	virtual void handleAsyncUpdate() override;

	void childStructureChanged(ControllableContainer*) override;



	// Inherited via DashboardItemProvider
	virtual juce::PopupMenu getDashboardCreateMenu(int idOffset) override;
	virtual DashboardItem* getDashboardItemFromMenuResult(int result) override;

};

static const juce::String lastFileListKey = "recentFiles";