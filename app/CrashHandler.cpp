/*
  ==============================================================================

	CrashHandler.cpp
	Created: 29 Oct 2017 1:40:04pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"
#include "CrashHandler.h"

#if JUCE_WINDOWS
#include <windows.h> 
#include <DbgHelp.h>
#include <tchar.h>
#endif

#if JUCE_WINDOWS
LONG WINAPI handleCrashStatic(LPEXCEPTION_POINTERS e);
void createDumpAndStrackTrace(void* e, File dumpFile, File traceFile);
#else
void handleCrashStatic(int signum);
void createDumpAndStrackTrace(int signum, File dumpFile, File traceFile);
#endif

juce_ImplementSingleton(CrashDumpUploader)
OrganicApplication::MainWindow* getMainWindow();

CrashDumpUploader::CrashDumpUploader() :
	Thread("Crashdump"),
	progress("Upload Progress", "", 0, 0, 1)
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
void handleCrashStatic(int e)
#endif
{

#if JUCE_WINDOWS
	CrashDumpUploader::getInstance()->handleCrash(e);
	return EXCEPTION_EXECUTE_HANDLER;
#else
	CrashDumpUploader::getInstance()->handleCrash(e);
#endif
}

#if JUCE_WINDOWS
void CrashDumpUploader::handleCrash(void* e)
#else
void CrashDumpUploader::handleCrash(int e)
#endif
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


	LOGERROR("A Crash happened !");
	LOGERROR(SystemStats::getStackBacktrace());

	//Let the app handle app-specific actions for crash
	getApp().handleCrashed();


	crashAction = GlobalSettings::getInstance()->actionOnCrash->getValueDataAsEnum<GlobalSettings::CrashAction>();


	traceFile = recoveredFile.getParentDirectory().getChildFile("crashlog.txt");

#if JUCE_WINDOWS
	dumpFile = recoveredFile.getParentDirectory().getChildFile("crashlog.dmp");
#else
	dumpFile = File();
#endif

	if (traceFile.existsAsFile()) traceFile.deleteFile();
	if (dumpFile.existsAsFile()) dumpFile.deleteFile();

	createDumpAndStrackTrace(e, dumpFile, traceFile);


	if (getApp().useWindow && crashAction == GlobalSettings::REPORT)
	{
		w.reset(new UploadWindow());
		DialogWindow::showDialog("Got crashed ?", w.get(), getMainWindow(), Colours::black, true);

		MessageManager::getInstance()->runDispatchLoop();

		exitApp();

	}
	else
	{
		if (GlobalSettings::getInstance()->autoSendCrashLog->boolValue())
		{
			LOG("Uploading crash...");
			uploadCrash();
		}

		exitApp();
	}
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
		.withParameter("email", contactEmail.isNotEmpty() ? contactEmail : "")
#if JUCE_DEBUG
		.withParameter("branch", "debug")
#else
		.withParameter("branch", getAppVersion().containsChar('b') ? "beta" : "stable")
#endif
		;

	if (dumpFile.existsAsFile())
	{
		LOG("Attaching dumpFile " << dumpFile.getFullPathName());
		url = url.withFileToUpload("dumpFile", dumpFile, "application/octet-stream");
	}

	if (traceFile.existsAsFile())
	{
		LOG("Attaching traceFile " << traceFile.getFullPathName());
		url = url.withFileToUpload("traceFile", traceFile, "application/octet-stream");
	}

	if (recoveredFile.existsAsFile())
	{
		LOG("Attaching sessionFile " << recoveredFile.getFullPathName());
		url = url.withFileToUpload("sessionFile", recoveredFile, "application/octet-stream");
	}

	std::function<bool(int, int)> callbackFunc = std::bind(&CrashDumpUploader::openStreamProgressCallback, this, std::placeholders::_1, std::placeholders::_2);
	int statusCode = 0;

	URL::InputStreamOptions options = URL::InputStreamOptions(URL::ParameterHandling::inPostData)
		.withExtraHeaders("Cache-Control: no-cache")
		.withProgressCallback(callbackFunc)
		.withStatusCode(&statusCode)
		.withConnectionTimeoutMs(5000)
		;

	std::unique_ptr<InputStream> stream(URL(url).createInputStream(options));

	bool failed = stream == nullptr;

#if JUCE_WINDOWS
	if (statusCode != 200) failed = true;
#endif

	if (failed)
	{
		LOGWARNING("Failed to connect, status code = " + String(statusCode));

		if (w != nullptr)
		{
			if (DialogWindow* dw = w->findParentComponentOfClass<DialogWindow>()) dw->exitModalState(0);
			MessageManagerLock mmLock;
			w->removeFromDesktop();
			w.reset();
		}

		return;
	}

	String convertedData = stream->readEntireStreamAsString();

#if JUCE_DEBUG
	LOG("Received : " << convertedData);
#endif

	if (convertedData.contains("error"))
	{
		LOGWARNING("Error during upload : " << convertedData);
	}
	else if (convertedData.contains("ok"))
	{
		LOG("Crash log uploaded succesfully");
		//crashFile.deleteFile();
	}
	else
	{
		LOGWARNING("Unknown message from crash log server " << convertedData << " (code " << String(statusCode) << ")");
	}

	sleep(300);

	if (w != nullptr)
	{
		if (DialogWindow* dw = w->findParentComponentOfClass<DialogWindow>()) dw->exitModalState(0);
		MessageManagerLock mmLock;
		w->removeFromDesktop();
		w.reset();
	}

}

bool CrashDumpUploader::openStreamProgressCallback(int bytesDownloaded, int totalLength)
{
	progress.setValue(bytesDownloaded * 1.0f / totalLength);
	LOG("Progress " << (int)(progress.floatValue() * 100) << "%");
	return !threadShouldExit();
}

void CrashDumpUploader::exitApp()
{
	File curFile;

	if (Engine::mainEngine != nullptr)
	{
		curFile = Engine::mainEngine->getFile();
		Engine::mainEngine->clear();
	}

	if (crashAction == GlobalSettings::RECOVER && recoveredFile.exists())
	{
		File::getSpecialLocation(File::currentApplicationFile).startAsProcess("-c \"" + recoveredFile.getFullPathName() + "\"");
	}
	else if (crashAction == GlobalSettings::REOPEN && curFile.exists())
	{
		File::getSpecialLocation(File::currentApplicationFile).startAsProcess("-c \"" + curFile.getFullPathName() + "\"");
	}

	getApp().quit();
}

void CrashDumpUploader::run()
{
	uploadCrash();
	MessageManager::getInstance()->stopDispatchLoop();
}

#if JUCE_WINDOWS
void createDumpAndStrackTrace(void* ev, File dumpFile, File traceFile)
#else
void createDumpAndStrackTrace(int signum, File dumpFile, File traceFile)
#endif
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
	cancelBT("Close only"),
	autoReopenBT("Send and recover"),
	recoverOnlyBT("Recover Only"),
	progressUI(&CrashDumpUploader::getInstance()->progress)
{
	okBT.addListener(this);
	addAndMakeVisible(&okBT);

	cancelBT.addListener(this);
	addAndMakeVisible(&cancelBT);

#if !JUCE_MAC
	recoverOnlyBT.addListener(this);
	addAndMakeVisible(&recoverOnlyBT);

	autoReopenBT.addListener(this);
	addAndMakeVisible(&autoReopenBT);
#endif

	addAndMakeVisible(&progressUI);

	mail.setMultiLine(false);
	mail.setColour(mail.backgroundColourId, BG_COLOR.brighter(.3f));
	mail.setColour(mail.textColourId, TEXT_COLOR.brighter());
	mail.setColour(mail.outlineColourId, BG_COLOR.brighter(.6f));
	mail.setText(GlobalSettings::getInstance()->crashContactEmail->stringValue(), dontSendNotification);
	mail.setTextToShowWhenEmpty("Your contact email if you accept to be contacted to help fix the problem (and only that !).", TEXT_COLOR);

	addAndMakeVisible(mail);

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
	g.drawImage(CrashDumpUploader::getInstance()->crashImage, getLocalBounds().toFloat());
}

void CrashDumpUploader::UploadWindow::resized()
{
	juce::Rectangle<int> r = getLocalBounds().removeFromBottom(getHeight() / 2);
	juce::Rectangle<int> br = r.removeFromBottom(30).reduced(2);
#if !JUCE_MAC
	autoReopenBT.setBounds(br.removeFromRight(100));
	br.removeFromRight(8);
	recoverOnlyBT.setBounds(br.removeFromRight(100));
	br.removeFromRight(8);
#endif
	okBT.setBounds(br.removeFromRight(100));
	br.removeFromRight(8);
	cancelBT.setBounds(br.removeFromRight(100));

	progressUI.setBounds(r.removeFromBottom(30).reduced(20, 5));

	mail.setBounds(r.removeFromTop(30).reduced(20, 0));

	editor.setBounds(r.reduced(20));
}

void CrashDumpUploader::UploadWindow::buttonClicked(Button* bt)
{
	okBT.setEnabled(false);
	cancelBT.setEnabled(false);

#if !JUCE_MAC
	autoReopenBT.setEnabled(false);
	recoverOnlyBT.setEnabled(false);
#endif

	CrashDumpUploader::getInstance()->uploadFile = bt == &autoReopenBT || bt == &okBT;
	CrashDumpUploader::getInstance()->crashMessage = editor.getText();
	CrashDumpUploader::getInstance()->contactEmail = mail.getText();
	CrashDumpUploader::getInstance()->crashAction = (bt == &autoReopenBT || bt == &recoverOnlyBT) ? GlobalSettings::RECOVER : GlobalSettings::KILL;


	if (mail.getText().isNotEmpty() && mail.getText() != GlobalSettings::getInstance()->crashContactEmail->stringValue())
	{
		GlobalSettings::getInstance()->crashContactEmail->setValue(mail.getText());
		getApp().saveGlobalSettings();
	}

	if (bt == &autoReopenBT || bt == &okBT)
	{
		CrashDumpUploader::getInstance()->startThread();
	}
	else if (bt == &cancelBT || bt == &recoverOnlyBT)
	{
		if (DialogWindow* dw = findParentComponentOfClass<DialogWindow>()) dw->exitModalState(0);
		MessageManager::getInstance()->stopDispatchLoop();
	}


	/*if (DialogWindow* dw = findParentComponentOfClass<DialogWindow>())
		dw->exitModalState(0);
		*/
}
