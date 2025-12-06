#include "JuceHeader.h"

juce_ImplementSingleton(FileDownloader)

FileDownloader::FileDownloader() :
	Thread("FileDownloader")
{
	progressParam.reset(new FloatParameter("Progress", "Download Progress", 0.0f, 0.0f, 1.0f));
}

FileDownloader::~FileDownloader()
{
}


void FileDownloader::addDownloadFile(juce::URL url, juce::File destinationFile, std::function<void(bool)> onFinished, bool overwriteExisting)
{
	filesToDownload.add(new FileToDownload(url, destinationFile, onFinished, overwriteExisting));
	startThread();
}

void FileDownloader::run()
{
	
	while (!threadShouldExit() && filesToDownload.size() > 0)
	{
		auto fileToDownload = filesToDownload.removeAndReturn(0);
		if (fileToDownload->destinationFile.existsAsFile() && !fileToDownload->overwriteExisting)
		{
			if (fileToDownload->onFinished)
				fileToDownload->onFinished(true);
			continue;
		}
		auto downloadTask = fileToDownload->url.downloadToFile(fileToDownload->destinationFile, URL::DownloadTaskOptions());
		if (downloadTask)
		{
			while (!downloadTask->isFinished() && !threadShouldExit())
			{
				float value = (float)downloadTask->getLengthDownloaded() / (float)downloadTask->getTotalLength();
				progressParam->setValue(value);
				wait(100);
			}
			bool success = !downloadTask->hadError();
			if (fileToDownload->onFinished)
				fileToDownload->onFinished(success);
		}
		else
		{
			if (fileToDownload->onFinished)
				fileToDownload->onFinished(false);
		}
	}

	progressParam->setValue(0.0f);
}


FileDownloaderUI::FileDownloaderUI()
{
	progressSlider.reset(FileDownloader::getInstance()->progressParam->createSlider());
	FileDownloader::getInstance()->progressParam->addAsyncParameterListener(this);

	addChildComponent(progressSlider.get());
}

FileDownloaderUI::~FileDownloaderUI()
{
	if (FileDownloader::getInstanceWithoutCreating())
	{
		FileDownloader::getInstance()->progressParam->removeAsyncParameterListener(this);
	}
}

void FileDownloaderUI::resized()
{
	progressSlider->setBounds(getLocalBounds());
}

void FileDownloaderUI::newMessage(const Parameter::ParameterEvent& e)
{
	if (e.type == Parameter::ParameterEvent::VALUE_CHANGED)
	{
		progressSlider->setVisible(progressSlider->parameter->floatValue() > 0.0f);
	}
}
