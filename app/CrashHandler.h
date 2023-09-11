#pragma once

class CrashDumpUploader : 
	public juce::Thread
{
public:
	juce_DeclareSingleton(CrashDumpUploader, true);

	CrashDumpUploader();
	~CrashDumpUploader();

	juce::URL remoteURL;
	juce::Image crashImage;

	bool doUpload;
	bool uploadFile;
	GlobalSettings::CrashAction crashAction;

	juce::File traceFile;
	juce::File dumpFile;
	juce::File recoveredFile;
	juce::String contactEmail;
	juce::String crashMessage;
	FloatParameter progress;

	void init(const juce::String& url, juce::Image image);

#if JUCE_WINDOWS
	void handleCrash(void * e);
#else
	void handleCrash(int signum);
#endif

	void run();
	void uploadCrash();

	bool openStreamProgressCallback(int /*bytesSent*/, int /*totalBytes*/);

	void exitApp();

	class UploadWindow :
		public juce::Component,
		public juce::Button::Listener
	{
	public:
		UploadWindow();
		~UploadWindow();

		juce::TextEditor mail;
		juce::TextEditor editor;
		juce::Image* image;
		juce::TextButton okBT;
		juce::TextButton cancelBT;
		juce::TextButton autoReopenBT;
		juce::TextButton recoverOnlyBT;
		FloatSliderUI progressUI;

		juce::Rectangle<int> imageRect;

		void paint(juce::Graphics& g) override;
		void resized() override;
		
		void buttonClicked(juce::Button* bt) override;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UploadWindow)
	};

	std::unique_ptr<UploadWindow> w;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrashDumpUploader)

};