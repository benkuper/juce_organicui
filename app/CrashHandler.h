#pragma once


class CrashDumpUploader : public Thread
{
public:
	juce_DeclareSingleton(CrashDumpUploader, true);

	CrashDumpUploader();
	~CrashDumpUploader();

	URL remoteURL;
	void init();

	void uploadDump();
	void run();

};