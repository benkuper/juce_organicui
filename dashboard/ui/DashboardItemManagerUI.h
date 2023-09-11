/*
  ==============================================================================

    DashboardItemManagerUI.h
    Created: 23 Apr 2017 12:33:58pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemManagerUI :
	public BaseManagerViewUI<DashboardItemManager, DashboardItem, BaseItemMinimalUI<DashboardItem>>,
	public ContainerAsyncListener,
	public Parameter::AsyncListener
{
public:
	DashboardItemManagerUI(DashboardItemManager * manager);
	~DashboardItemManagerUI();

	juce::Image bgImage;
	bool tmpShowTools;

	static std::function<void(juce::PopupMenu*, int)> customAddItemsToMenuFunc;
	static std::function<void(int, int, DashboardItemManagerUI*, juce::Point<float>)> customHandleMenuResultFunc;

	virtual void resized() override;

	virtual void paint(juce::Graphics& g) override;
	virtual void paintOverChildren(juce::Graphics& g) override;

	virtual bool checkItemShouldBeVisible(BaseItemMinimalUI<DashboardItem>* se) override;

	bool isInterestedInDragSource(const SourceDetails & dragSourceDetails) override;
	void itemDropped(const SourceDetails &details) override;

	void showMenuAndAddItem(bool fromAddButton, juce::Point<int> mousePos) override;

	BaseItemMinimalUI<DashboardItem> * createUIForItem(DashboardItem *) override;

	void newMessage(const ContainerAsyncEvent& e) override;
	void newMessage(const Parameter::ParameterEvent& e) override;
};