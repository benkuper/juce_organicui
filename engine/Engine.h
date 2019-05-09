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
	public FileBasedDocument,
	public AsyncUpdater,
	public ProgressNotifier,
	public DashboardItemProvider,
	public Timer //for auto save
{
public:
	Engine(const String &fileName = "File",const String &fileExtension = ".file");
	virtual ~Engine();

	static Engine * mainEngine;

	ScopedPointer<InspectableSelectionManager> selectionManager;


	bool isBetaVersion;
	int betaVersion;

	String convertURL = ""; //for file conversion

	String fileName = "File";
	String fileExtension = ".file";
	String fileWildcard = "*"+fileExtension;

	String lastFileAbsolutePath; //Used for checking in saveDocument if new file is different

	int autoSaveIndex;

	virtual void changed() override;
	void createNewGraph();
    void clear() override;
	virtual void clearInternal() {}; //to override to clear specific application classes

	void parseCommandline(const String &);

	//==============================================================================
	// see EngineFileDocument.cpp

	//  inherited from FileBasedDocument
	String getDocumentTitle()override;
	Result loadDocument(const File& file) override;
	Result saveDocument(const File& file) override;
	Result saveBackupDocument(int index);

	File getLastDocumentOpened() override;
	void setLastDocumentOpened(const File& file) override;

	//    #if JUCE_MODAL_LOOPS_PERMITTED
	//     File getSuggestedSaveAsFile (const File& defaultFile)override;
	//    #endif

	// our Saving methods
	var getJSONData() override;
	void loadJSONData(var data, ProgressTask * loadingTask);
	virtual void loadJSONDataInternalEngine(var data, ProgressTask * loadingTask) {}

	bool checkFileVersion(DynamicObject * metaData, bool checkForNewerVersion = false);
	bool versionIsNewerThan(String versionToCheck, String referenceVersion);
	int getBetaVersion(String version);
	virtual String getMinimumRequiredFileVersion();

	int64 loadingStartTime;

	void fileLoaderEnded();
	bool allLoadingThreadsAreEnded();
	void loadDocumentAsync(const File & file);

	virtual void timerCallback() override;


	class FileLoader : public Thread, public Timer {
	public:
		FileLoader(Engine * e, File f) :Thread("EngineLoader"), owner(e), fileToLoad(f) {
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
		Engine * owner;
		File fileToLoad;
		bool isEnded;


	};

	ScopedPointer<FileLoader> fileLoader;

	ListenerList<EngineListener> engineListeners;
	void addEngineListener(EngineListener* e) { engineListeners.add(e); }
	void removeEngineListener(EngineListener* e) { engineListeners.remove(e); }

	// ASYNC
	class  EngineEvent
	{
	public:
		enum Type { START_LOAD_FILE, FILE_PROGRESS, END_LOAD_FILE, FILE_SAVED, ENGINE_CLEARED , FILE_CHANGED };

		EngineEvent(Type t, Engine * engine) :
			type(t),
			engine(engine)
		{
		}

		Type type;
		Engine * engine;
	};

	QueuedNotifier<EngineEvent> engineNotifier;
	typedef QueuedNotifier<EngineEvent>::Listener AsyncListener;

	void addAsyncEngineListener(AsyncListener* newListener) { engineNotifier.addListener(newListener); }
	void addAsyncCoalescedEngineListener(AsyncListener* newListener) { engineNotifier.addAsyncCoalescedListener(newListener); }
	void removeAsyncEngineListener(AsyncListener* listener) { engineNotifier.removeListener(listener); }

	bool isLoadingFile;
	bool isClearing;
	var jsonData;

	virtual void handleAsyncUpdate()override;

	void childStructureChanged(ControllableContainer *) override;



	// Inherited via DashboardItemProvider
	virtual PopupMenu getDashboardCreateMenu(int idOffset) override;
	virtual DashboardItem * getDashboardItemFromMenuResult(int result) override;

};

static String lastFileListKey = "recentFiles";