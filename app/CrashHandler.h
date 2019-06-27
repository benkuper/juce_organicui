#pragma once


class CrashDumpUploader : public Thread
{
public:
	juce_DeclareSingleton(CrashDumpUploader, true);

	CrashDumpUploader();
	~CrashDumpUploader();

	File crashFile;
	URL remoteURL;
	bool crashFound;

	bool init(); //returns true if a crashlog is found

	void uploadDump();
	void run();

};