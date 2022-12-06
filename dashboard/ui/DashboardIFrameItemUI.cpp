/*
  ==============================================================================

	DashboardIFrameItem.cpp
	Created: 5 Dec 2021 4:43:49pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

DashboardIFrameItemUI::DashboardIFrameItemUI(DashboardIFrameItem* item) :
	DashboardItemUI(item),
	iFrameItem(item)
#if JUCE_WINDOWS
	, web(false, File(), File::getSpecialLocation(File::windowsLocalAppData).getChildFile("Chataigne"))
#endif

	//,web(WebBrowserComponent::Options().withBackend(WebBrowserComponent::Options::Backend::webview2).withWinWebView2Options(
	//	WebBrowserComponent::Options::WinWebView2().withUserDataFolder(File::getSpecialLocation(File::windowsLocalAppData).getChildFile("Chataigne")).withStatusBarDisabled())) //7.0.3
{

#if JUCE_WINDOWS
	if (!GlobalSettings::getInstance()->useGLRenderer->boolValue())
	{
		addChildComponent(web);
	}
	else
	{
		NLOGWARNING(item->niceName, "This app is using OpenGL renderer.\nIf you want to see the webpage inside the app, please disable OpenGL Renderer in the Preferences and restart the app.");
		item->setWarningMessage("Webpage on windows needs to not use OpenGL. Change it in Preferences.");
	}
#else
	addChildComponent(web);
#endif

	MessageManagerLock mmLock;
	web.setVisible(!DashboardManager::getInstance()->editMode->boolValue());
	web.goToURL(iFrameItem->url->stringValue());
	resized();
}

DashboardIFrameItemUI::~DashboardIFrameItemUI()
{
}


void DashboardIFrameItemUI::paint(Graphics& g)
{
	g.fillAll(Colours::black.withAlpha(.5f));
	g.setColour(Colours::white.withAlpha(.6f));
	g.drawFittedText(iFrameItem->url->stringValue(), getLocalBounds(), Justification::centred, 1);
}

void DashboardIFrameItemUI::resized()
{
	DashboardItemUI::resized();
	web.setBounds(getLocalBounds());
}

void DashboardIFrameItemUI::updateEditModeInternal(bool editMode)
{
	web.setVisible(!editMode);
}


void DashboardIFrameItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	DashboardItemUI::controllableFeedbackUpdateInternal(c);
	if (c == iFrameItem->url) web.goToURL(iFrameItem->url->stringValue());
}