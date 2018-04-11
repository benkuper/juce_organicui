/*
  ==============================================================================

    Main.h
    Created: 25 Oct 2016 11:16:59pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef ORGANICAPPLICATION_H_INCLUDED
#define ORGANICAPPLICATION_H_INCLUDED


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
	public AppUpdater::AsyncListener
{
public:
	//==============================================================================
	OrganicApplication(const String & appName);

	ControllableContainer appSettings;

	ApplicationCommandManager commandManager;
	ScopedPointer<ApplicationProperties> appProperties;

	ScopedPointer<Engine> engine;
	OrganicMainContentComponent * mainComponent;

	const String getApplicationName() override;
	const String getApplicationVersion() override;
	virtual bool moreThanOneInstanceAllowed() override { return true; }

	void initialise(const String& /*commandLine*/) override;
	virtual void initialiseInternal(const String &) = 0;

	virtual void shutdown() override;
	virtual void systemRequestedQuit() override;
	virtual void anotherInstanceStarted(const String& commandLine) override;

	virtual void newMessage(const Engine::EngineEvent &e) override;
	virtual void newMessage(const AppUpdater::UpdateEvent &e) override;


	virtual void updateAppTitle();

	class MainWindow : public DocumentWindow
	{		
	public:
		MainWindow(String name, OrganicMainContentComponent * mainComponent);

		void closeButtonPressed() override;


		OrganicMainContentComponent * mainComponent;
		void visibilityChanged() override;

#if JUCE_OPENGL
		OpenGLContext openGLContext;
#endif

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

	ScopedPointer<MainWindow> mainWindow;
};


#endif  // ORGANICAPPLICATION_H_INCLUDED
