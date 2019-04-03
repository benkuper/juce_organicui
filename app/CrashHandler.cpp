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
LONG WINAPI createMiniDump(LPEXCEPTION_POINTERS exceptionPointers);
#else
void createStackTrace(int signum);
#endif

String dumpFileString;
char * dumpFileName = (char *)"notset.dmp";


juce_ImplementSingleton(CrashDumpUploader)

CrashDumpUploader::CrashDumpUploader() :
	Thread("Crashdump")
{

}

CrashDumpUploader::~CrashDumpUploader()
{
}

void CrashDumpUploader::init()
{

#if JUCE_WINDOWS
	crashFile = File::getSpecialLocation(File::tempDirectory).getParentDirectory().getChildFile(getApp().getApplicationName()+"_crash.dmp");
    SystemStats::setApplicationCrashHandler((SystemStats::CrashHandlerFunction)createMiniDump);
#else
    crashFile = File::getSpecialLocation(File::tempDirectory).getParentDirectory().getChildFile(getApp().getApplicationName()+"_crash.txt");
    SystemStats::setApplicationCrashHandler((SystemStats::CrashHandlerFunction)createStackTrace);
#endif

    dumpFileString = crashFile.getFullPathName();
    dumpFileName = dumpFileString.getCharPointer().getAddress();
   
    if (crashFile.existsAsFile())
    {
        LOGWARNING("Crash log found, sending to Houston...");
        startThread();
    }

	
}

void CrashDumpUploader::uploadDump()
{
	if (remoteURL.isEmpty())
	{
		LOGWARNING("Crash dump upload url has not been assigned");
		crashFile.deleteFile();
		return;
	}

	URL url = remoteURL.withParameter("username", SystemStats::getFullUserName().replace(" ","-"))
		.withParameter("os", SystemStats::getOperatingSystemName().replace(" ","-"))
		.withParameter("version", getAppVersion())
#if JUCE_DEBUG
		.withParameter("branch", "debug")
#else
		.withParameter("branch", getAppVersion().containsChar('b') ? "beta" : "stable")
#endif
		.withFileToUpload("dumpfile", crashFile, "application/octet-stream");

	WebInputStream stream(url, true);

	String convertedData = stream.readEntireStreamAsString();

#if JUCE_DEBUG
	LOG("Received : " << convertedData);
#endif

	if (convertedData.contains("error"))
	{
		LOGWARNING("Error during upload");
	} else if(convertedData.contains("ok"))
	{
		LOG("Crash log uploaded succesfully");
		crashFile.deleteFile();
	} else
	{
		LOGWARNING("Unknown message from crash log server");
	}
}

void CrashDumpUploader::run()
{
	uploadDump();
}


#if JUCE_WINDOWS 

LONG WINAPI createMiniDump(LPEXCEPTION_POINTERS exceptionPointers)
{

	HANDLE hFile = CreateFile(dumpFileName, GENERIC_READ | GENERIC_WRITE,
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
	} else
	{
		_tprintf(_T("CreateFile failed. Error: %u \n"), GetLastError());
	}
	 
	int selectedButtonId = MessageBox(nullptr,
		_T("Oh, no ! Chataigne got very sad and died alone. Would you help find find out why ? If you click Yes, the file will automatically be uploaded at the next launch of Chataigne. If not, nothing will be done and this sad event will be forever forgotten."),
		_T("Sacré Hubert, toujours le mot pour rire !"),
		MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1
	);

	_tprintf(_T("User pushed : %d \n"), selectedButtonId);

	if (selectedButtonId == IDNO)
		DeleteFile(dumpFileName);
	
	return EXCEPTION_EXECUTE_HANDLER;
}


#else

void createStackTrace(int signum)
{
    DBG("Create Stack trace here !");
    String stackTrace = SystemStats::getStackBacktrace();
    
    DBG("Stack trace : " << stackTrace);
    
    File f(dumpFileString);
    FileOutputStream fos(f);
    if (fos.openedOk())
    {
        fos.writeText(stackTrace,false,false,"\n");
        fos.flush();
    }
    
    DBG("Written to file : " << f.getFullPathName());
}

#endif
