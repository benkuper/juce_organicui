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
	public juce::JUCEApplication,
	public Engine::AsyncListener,
	public AppUpdaterAsyncListener
{
public:
	//==============================================================================
	OrganicApplication(const juce::String& appName, bool useWindow = true, const juce::Image& trayIconImage = juce::Image());

	ControllableContainer appSettings;

	juce::ApplicationCommandManager commandManager;
	std::unique_ptr<juce::ApplicationProperties> appProperties;

	std::unique_ptr<Engine> engine;
	std::unique_ptr<OrganicMainContentComponent> mainComponent;

	bool useWindow;
	juce::Image trayIconImage;
	bool launchedFromCrash = false;

	const juce::String getApplicationName() override;
	const juce::String getApplicationVersion() override;
	virtual bool moreThanOneInstanceAllowed() override;

	void initialise(const juce::String& /*commandLine*/) override;
	virtual void initialiseInternal(const juce::String&) = 0;
	virtual void afterSettingsLoaded() {}
	virtual void afterInit() {}

	virtual void shutdown() override;
	virtual void systemRequestedQuit() override;
	virtual void anotherInstanceStarted(const juce::String& commandLine) override;

	virtual void handleCrashed() {}

	virtual void newMessage(const Engine::EngineEvent &e) override;
	virtual void newMessage(const AppUpdateEvent &e) override;

	virtual void clearGlobalSettings();
	virtual void saveGlobalSettings();

	virtual void updateAppTitle();
	virtual juce::Component* getMenuBarComponent();

	class TrayIcon :
		public juce::SystemTrayIconComponent
	{
	public:
		TrayIcon(const juce::Image& iconImage);
		~TrayIcon();

		void mouseDown(const juce::MouseEvent& e) override;

		class  TrayIconListener
		{
		public:
			/** Destructor. */
			virtual ~TrayIconListener() {}
			virtual void trayIconMouseDown(const juce::MouseEvent& e) = 0;
		};

		juce::ListenerList<TrayIconListener> trayIconListeners;
		void addTrayIconListener(TrayIconListener* newListener) { trayIconListeners.add(newListener); }
		void removeTrayIconListener(TrayIconListener* listener) { trayIconListeners.remove(listener); }
	};

	class MainWindow :
		public juce::DocumentWindow,
		public TrayIcon::TrayIconListener
	{		
	public:
		MainWindow(juce::String name, OrganicMainContentComponent * mainComponent, const juce::Image &image);


		OrganicMainContentComponent * mainComponent;
		std::unique_ptr<TrayIcon> trayIcon;
		juce::Image iconImage;


		void closeToTray();
		void openFromTray();

		void closeButtonPressed() override;

		virtual void trayIconMouseDown(const juce::MouseEvent& e) override;

		void setTrayIconVisible(bool visible);
		virtual void showTrayMenu();
		virtual void addTrayMenuOptions(const juce::PopupMenu& menu) {};
		virtual void handlTrayMenuResult(int result) {}


		void visibilityChanged() override;


	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

	std::unique_ptr<MainWindow> mainWindow;
};

static OrganicApplication& getApp() { return *dynamic_cast<OrganicApplication*>(juce::JUCEApplication::getInstance()); }
