#pragma once


class CrashDumpUploader : public Thread
{
public:
	juce_DeclareSingleton(CrashDumpUploader, true);

	CrashDumpUploader();
	~CrashDumpUploader();

	File crashFile;
	URL remoteURL;
	Image crashImage;

	bool uploadEnabled;
	bool crashFound;

	String crashMessage;


	bool init(bool autoUpload = true, bool showWindow = true); //returns true if a crashlog is found

	void uploadDump();
	void run();


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

		juce::Rectangle<int> imageRect;

		void paint(Graphics& g) override;
		void resized() override;
		
		void buttonClicked(Button* bt) override;
	};
};