/*
  ==============================================================================

	DashboardIFrameItem.cpp
	Created: 5 Dec 2021 4:43:49pm
	Author:  bkupe

  ==============================================================================
*/

DashboardIFrameItemUI::DashboardIFrameItemUI(DashboardIFrameItem* item) :
	DashboardItemUI(item),
	iFrameItem(item)
#if JUCE_WINDOWS
	, web(false, WebView2Preferences())
#endif
{
	addChildComponent(web);
	web.goToURL(iFrameItem->url->stringValue());

	web.setVisible(!DashboardManager::getInstance()->editMode->boolValue());
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
	web.setBounds(getLocalBounds());
}

void DashboardIFrameItemUI::updateEditModeInternal(bool editMode)
{
	web.setVisible(!editMode);
}


void DashboardIFrameItemUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if(c == iFrameItem->url) web.goToURL(iFrameItem->url->stringValue());
}