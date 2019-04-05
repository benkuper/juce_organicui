/*
  ==============================================================================

    Main.h
    Created: 25 Oct 2016 11:16:59pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

/*
==============================================================================

This file was auto-generated!

It contains the basic startup code for a Juce application.

==============================================================================
*/

#pragma warning(disable:4244 4100 4305)

#if JUCE_WINDOWS_
#include <windows.h>
LONG WINAPI createMiniDump(LPEXCEPTION_POINTERS exceptionPointers);
#endif


//==============================================================================
class OrganicApplication :
	public JUCEApplication,
	public Engine::AsyncListener,
	public AppUpdaterAsyncListener
{
public:
	//==============================================================================
	OrganicApplication(const String & appName, bool useWindow = true);

	ControllableContainer appSettings;

	ApplicationCommandManager commandManager;
	ScopedPointer<ApplicationProperties> appProperties;

	ScopedPointer<Engine> engine;
	ScopedPointer<OrganicMainContentComponent> mainComponent;

	bool useWindow;

	const String getApplicationName() override;
	const String getApplicationVersion() override;
	virtual bool moreThanOneInstanceAllowed() override { return true; }

	void initialise(const String& /*commandLine*/) override;
	virtual void initialiseInternal(const String &) = 0;
	virtual void afterInit() {}

	virtual void shutdown() override;
	virtual void systemRequestedQuit() override;
	virtual void anotherInstanceStarted(const String& commandLine) override;

	virtual void newMessage(const Engine::EngineEvent &e) override;
	virtual void newMessage(const AppUpdateEvent &e) override;


	virtual void updateAppTitle();

	class MainWindow : public DocumentWindow
	{		
	public:
		MainWindow(String name, OrganicMainContentComponent * mainComponent);

		void closeButtonPressed() override;


		OrganicMainContentComponent * mainComponent;
		void visibilityChanged() override;


	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

	ScopedPointer<MainWindow> mainWindow;
};



