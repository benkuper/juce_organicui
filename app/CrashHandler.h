#pragma once


class CrashDumpUploader : public Thread
{
public:
	juce_DeclareSingleton(CrashDumpUploader, true);

	CrashDumpUploader();
	~CrashDumpUploader();

	URL remoteURL;
	Image crashImage;

	bool doUpload;
	bool autoReopen;

	File traceFile;
	File dumpFile;
	File recoveredFile;
	String crashMessage;

	void init(const String& url, Image image);

	void handleCrash(void * e);

	void run();
	void uploadCrash();


	class UploadWindow :
		public Component,
		public Button::Listener
	{
	public:
		UploadWindow();
		~UploadWindow();

		TextEditor editor;
		Image* image;
		TextButton okBT;
		TextButton cancelBT;
		TextButton autoReopenBT;

		juce::Rectangle<int> imageRect;

		void paint(Graphics& g) override;
		void resized() override;
		
		void buttonClicked(Button* bt) override;
	};
};