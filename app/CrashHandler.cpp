/*
  ==============================================================================

    CrashHandler.cpp
    Created: 29 Oct 2017 1:40:04pm
    Author:  Ben

  ==============================================================================
*/

#include "CrashHandler.h"

#if JUCE_WINDOWS
#include <windows.h> 
#include <DbgHelp.h>
#include <tchar.h>
#endif

#if JUCE_WINDOWS
LONG WINAPI handleCrashStatic(LPEXCEPTION_POINTERS e);
#else
void handleCrashStatic(int signum);
#endif

void createDumpAndStrackTrace(void* e, File dumpFile, File traceFile);


juce_ImplementSingleton(CrashDumpUploader)
OrganicApplication::MainWindow* getMainWindow();

CrashDumpUploader::CrashDumpUploader() :
	Thread("Crashdump"),
	doUpload(true)
{

}

CrashDumpUploader::~CrashDumpUploader()
{
}

void CrashDumpUploader::init(const String& url, Image image)
{
	remoteURL = URL(url);
	crashImage = image;

	SystemStats::setApplicationCrashHandler((SystemStats::CrashHandlerFunction)handleCrashStatic);
}

#if JUCE_WINDOWS
LONG WINAPI handleCrashStatic(LPEXCEPTION_POINTERS e)
#else
void handleCrashStatic(int signum)
#endif
{
	CrashDumpUploader::getInstance()->handleCrash(e);

#if JUCE_WINDOWS
	return EXCEPTION_EXECUTE_HANDLER;
#endif
}

void CrashDumpUploader::handleCrash(void * e)
{
	//create recovered file 
	File f = Engine::mainEngine->getFile();

	recoveredFile = f.existsAsFile() ? f.getParentDirectory().getChildFile(f.getFileNameWithoutExtension() + "_recovered" + f.getFileExtension()) : File::getSpecialLocation(File::userDocumentsDirectory).getChildFile(getApp().appProperties->getStorageParameters().applicationName + "/recovered_session" + Engine::mainEngine->fileExtension);

	if (recoveredFile.existsAsFile()) recoveredFile.deleteFile();

	var data = Engine::mainEngine->getJSONData();
	std::unique_ptr<OutputStream> os(recoveredFile.createOutputStream());
	if (os != nullptr)
	{
		JSON::writeToStream(*os, data, false);
		os->flush();
	}


	//Let the app handle app-specific actions for crash
	getApp().handleCrashed();


	autoReopen = GlobalSettings::getInstance()->autoReopenFileOnCrash->boolValue();


	if (GlobalSettings::getInstance()->enableCrashUpload->boolValue())
	{
		traceFile = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("crashlog.txt");

#if JUCE_WINDOWS
		dumpFile = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("crashlog.dmp");
#else
		dumpFile = File();
#endif

		if (traceFile.existsAsFile()) traceFile.deleteFile();
		if (dumpFile.existsAsFile()) dumpFile.deleteFile();

		createDumpAndStrackTrace(e, dumpFile, traceFile);

		/*
		if (shoudShowWindowOnCrash)
		{
			autoReopen = AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, "Oops, I did it again...", "Well, I crashed.\nIt was bound to happen at some point, right ?\nMaybe time to think about the meaning of all this, the meaning of life, make yourself a smoothie...\nYou can even start exactly where you left if you click the \"Yes\" button.\nDon't worry, in any case I will keep your baby safe and create a crash backup file.", "Yes", "No");
		}
		*/

		doUpload = true; //by default, unless hit cancel

		UploadWindow w;
		DialogWindow::showModalDialog("Crash found !", &w, getMainWindow(), Colours::black, true);

		if (doUpload)
		{
			uploadCrash();
		}
	}

	if (autoReopen && recoveredFile.exists())
	{
		File::getSpecialLocation(File::currentApplicationFile).startAsProcess("-c " + recoveredFile.getFullPathName());
	}

	getApp().quit();
}

void CrashDumpUploader::uploadCrash()
{
	if (remoteURL.isEmpty())
	{
		LOGWARNING("Crash dump upload url has not been assigned");
		dumpFile.deleteFile();
		traceFile.deleteFile();
		return;
	}

	URL url = remoteURL.withParameter("username", SystemStats::getFullUserName().replace(" ", "-"))
		.withParameter("os", SystemStats::getOperatingSystemName().replace(" ", "-"))
		.withParameter("version", getAppVersion())
		.withParameter("message", crashMessage.isNotEmpty() ? crashMessage : "No message")
#if JUCE_DEBUG
		.withParameter("branch", "debug")
#else
		.withParameter("branch", getAppVersion().containsChar('b') ? "beta" : "stable")
#endif
		;

	if (dumpFile.existsAsFile())
	{
		url = url.withFileToUpload("dumpFile", dumpFile, "application/octet-stream");
	}

	if (traceFile.existsAsFile())
	{
		url = url.withFileToUpload("traceFile", traceFile, "application/octet-stream");
	}

	if (recoveredFile.existsAsFile())
	{
		url = url.withFileToUpload("sessionFile", recoveredFile, "application/octet-stream");
	}

	WebInputStream stream(url, true);

	String convertedData = stream.readEntireStreamAsString();

#if JUCE_DEBUG
	LOG("Received : " << convertedData);
#endif

	if (convertedData.contains("error"))
	{
		LOGWARNING("Error during upload");
	}
	else if (convertedData.contains("ok"))
	{
		LOG("Crash log uploaded succesfully");
		//crashFile.deleteFile();
	}
	else
	{
		LOGWARNING("Unknown message from crash log server " << convertedData << " (code " << String(stream.getStatusCode()) << ")");
	}
}

void CrashDumpUploader::run()
{
	uploadCrash();
}

void createDumpAndStrackTrace(void * ev, File dumpFile, File traceFile)
{

#if JUCE_WINDOWS

	LPEXCEPTION_POINTERS exceptionPointers = (LPEXCEPTION_POINTERS)ev;

	HANDLE hFile = CreateFile(dumpFile.getFullPathName().getCharPointer(), GENERIC_READ | GENERIC_WRITE,
		0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != nullptr && hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION exceptionInformation;

		exceptionInformation.ThreadId = GetCurrentThreadId();
		exceptionInformation.ExceptionPointers = exceptionPointers;
		exceptionInformation.ClientPointers = FALSE;

		MINIDUMP_TYPE dumpType = MiniDumpNormal;

		BOOL dumpWriteResult = MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			dumpType,
			exceptionPointers != nullptr ? &exceptionInformation : 0,
			nullptr,
			nullptr);

		if (!dumpWriteResult)
			_tprintf(_T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError());
		else
			_tprintf(_T("Minidump created.\n"));

		CloseHandle(hFile);
	}
	else
	{
		_tprintf(_T("CreateFile failed. Error: %u \n"), GetLastError());
	}
#endif


	//Stack trace

    DBG("Create Stack trace here !");
    String stackTrace = SystemStats::getStackBacktrace();
    
    FileOutputStream fos(traceFile);
	if (fos.openedOk())
	{
		fos.writeText(stackTrace, false, false, "\n");
		fos.flush();
	}
}


CrashDumpUploader::UploadWindow::UploadWindow() :
	okBT("Send and close"),
	cancelBT("Cancel"),
	autoReopenBT("Send and recover")
{
	okBT.addListener(this);
	addAndMakeVisible(&okBT);

	cancelBT.addListener(this);
	addAndMakeVisible(&cancelBT);

	autoReopenBT.addListener(this);
	addAndMakeVisible(&autoReopenBT);

	addAndMakeVisible(&editor);
	editor.setColour(editor.backgroundColourId, BG_COLOR.brighter(.3f));
	editor.setColour(editor.textColourId, TEXT_COLOR.brighter());
	editor.setColour(editor.outlineColourId, BG_COLOR.brighter(.6f));
	editor.setTextToShowWhenEmpty("Your super comprehensive yet fun explanation here. You can write love messages as well, but only if you mean it.", TEXT_COLOR);
	editor.setMultiLine(true);
	editor.setReturnKeyStartsNewLine(true);

	
	setSize(800, 600);

}

CrashDumpUploader::UploadWindow::~UploadWindow()
{

}

void CrashDumpUploader::UploadWindow::paint(Graphics& g)
{
	g.drawImage(CrashDumpUploader::getInstance()->crashImage,getLocalBounds().toFloat());
}

void CrashDumpUploader::UploadWindow::resized()
{
	juce::Rectangle<int> r = getLocalBounds().removeFromBottom(getHeight() / 2);
	juce::Rectangle<int> br = r.removeFromBottom(30).reduced(2);
	autoReopenBT.setBounds(br.removeFromRight(100));
	br.removeFromRight(8); 
	okBT.setBounds(br.removeFromRight(100));
	br.removeFromRight(8);
	cancelBT.setBounds(br.removeFromRight(100));
	editor.setBounds(r.reduced(20));
}

void CrashDumpUploader::UploadWindow::buttonClicked(Button* bt)
{
	if (bt == &okBT || bt == &autoReopenBT)
	{
		CrashDumpUploader::getInstance()->crashMessage = editor.getText();
		CrashDumpUploader::getInstance()->doUpload = true;
		if (bt == &autoReopenBT) CrashDumpUploader::getInstance()->autoReopen = true;
	}
	else if (bt == &cancelBT)
	{
		CrashDumpUploader::getInstance()->doUpload = false;
	}

	if (DialogWindow* dw = findParentComponentOfClass<DialogWindow>())
		dw->exitModalState(0);
}
