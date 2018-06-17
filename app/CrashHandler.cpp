/*
  ==============================================================================

    CrashHandler.cpp
    Created: 29 Oct 2017 1:40:04pm
    Author:  Ben

  ==============================================================================
*/


#if JUCE_WINDOWS
#include <windows.h> 
#include <DbgHelp.h>
#include <tchar.h>

LONG WINAPI createMiniDump(LPEXCEPTION_POINTERS exceptionPointers);
#endif

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
	SystemStats::setApplicationCrashHandler((SystemStats::CrashHandlerFunction)createMiniDump);
#endif

	File f = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("crash.dmp");
	DBG("Check for crash dump file : "+f.getFullPathName());

	if (f.existsAsFile())
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
		return;
	}

	File f = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("crash.dmp");
	URL url = remoteURL.withParameter("username", SystemStats::getFullUserName())
		.withParameter("os", SystemStats::getOperatingSystemName())
		.withParameter("version", getAppVersion())
#if DEBUG
		.withParameter("branch", "debug")
#else
		.withParameter("branch", getAppVersion().endsWith("b") ? "beta" : "stable")
#endif
		.withFileToUpload("dumpfile", f, "application/octet-stream");

	WebInputStream stream(url, true);

	String convertedData = stream.readEntireStreamAsString();

#if DEBUG
	LOG("Received : " << convertedData);
#endif

	if (convertedData.contains("error"))
	{
		LOGWARNING("Error during upload");
	} else if(convertedData.contains("ok"))
	{
		LOG("Crash log uploaded succesfully");
		f.deleteFile();
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
	auto dumpFileName = "crash.dmp";

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
		_T("Oh, no ! Chataigne got very sad and died alone. Would you help find find out why ?"),
		_T("Sacré Hubert, toujours le mot pour rire !"),
		MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1
	);

	_tprintf(_T("User pushed : %d \n"), selectedButtonId);

	if (selectedButtonId == IDNO)
		DeleteFile(dumpFileName);

	return EXCEPTION_EXECUTE_HANDLER;
}


#endif
