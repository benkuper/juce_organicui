#pragma once


class CrashDumpUploader : public Thread
{
public:
	juce_DeclareSingleton(CrashDumpUploader, true);

	CrashDumpUploader();
	~CrashDumpUploader();

	File crashFile;
	URL remoteURL;

	void init();

	void uploadDump();
	void run();

};