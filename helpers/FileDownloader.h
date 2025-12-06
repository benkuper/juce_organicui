#pragma once


class FileDownloaderUI :
	public juce::Component,
	public Parameter::AsyncListener
{
public:
	FileDownloaderUI();
	~FileDownloaderUI();

	std::unique_ptr<FloatSliderUI> progressSlider;

	void resized() override;
	void newMessage(const Parameter::ParameterEvent& e) override;
};

class FileDownloader :
	public juce::Thread
{
public:

	juce_DeclareSingleton(FileDownloader, false);
	FileDownloader();
	~FileDownloader();

	std::unique_ptr<FloatParameter> progressParam;

	class FileToDownload
	{
	public:
		FileToDownload(juce::URL u, juce::File f, std::function<void(bool)> cb, bool o = true)
			: url(u), destinationFile(f), onFinished(cb), overwriteExisting(o) {
		}
		~FileToDownload() {}
		juce::URL url;
		juce::File destinationFile;
		bool overwriteExisting;
		
		std::function<void(bool success)> onFinished;
	};

	juce::OwnedArray<FileToDownload, juce::CriticalSection> filesToDownload;

	void addDownloadFile(juce::URL url, juce::File destinationFile, std::function<void(bool)> onFinished, bool overwriteExisting = true);

	void run() override;

};