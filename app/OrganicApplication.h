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
	OrganicApplication(const String & appName, bool useWindow = true, const Image &trayIconImage = Image());

	ControllableContainer appSettings;

	ApplicationCommandManager commandManager;
	std::unique_ptr<ApplicationProperties> appProperties;

	std::unique_ptr<Engine> engine;
	std::unique_ptr<OrganicMainContentComponent> mainComponent;

	bool useWindow;
	Image trayIconImage;

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

	virtual void clearGlobalSettings();

	virtual void updateAppTitle();

	class TrayIcon :
		public SystemTrayIconComponent
	{
	public:
		TrayIcon(const Image& iconImage);
		~TrayIcon();

		void mouseDown(const MouseEvent& e) override;

		class  TrayIconListener
		{
		public:
			/** Destructor. */
			virtual ~TrayIconListener() {}
			virtual void trayIconMouseDown(const MouseEvent& e) = 0;
		};

		ListenerList<TrayIconListener> trayIconListeners;
		void addTrayIconListener(TrayIconListener* newListener) { trayIconListeners.add(newListener); }
		void removeTrayIconListener(TrayIconListener* listener) { trayIconListeners.remove(listener); }
	};

	class MainWindow :
		public DocumentWindow,
		public TrayIcon::TrayIconListener
	{		
	public:
		MainWindow(String name, OrganicMainContentComponent * mainComponent, const Image &image);

		void closeButtonPressed() override;

		OrganicMainContentComponent * mainComponent;
		std::unique_ptr<TrayIcon> trayIcon;
		Image iconImage;

		virtual void trayIconMouseDown(const MouseEvent& e) override;

		void setTrayIconVisible(bool visible);
		virtual void showTrayMenu();
		virtual void addTrayMenuOptions(const PopupMenu& menu) {};
		virtual void handlTrayMenuResult(int result) {}

		void visibilityChanged() override;


	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

	std::unique_ptr<MainWindow> mainWindow;
};



