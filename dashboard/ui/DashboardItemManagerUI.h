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

	Image bgImage;
	bool tmpShowTools;

	static std::function<void(PopupMenu*, int)> customAddItemsToMenuFunc;
	static std::function<void(int, int, DashboardItemManagerUI*, Point<float>)> customHandleMenuResultFunc;

	virtual void resized() override;

	virtual void paint(Graphics& g) override;
	virtual void paintOverChildren(Graphics& g) override;

	bool isInterestedInDragSource(const SourceDetails & dragSourceDetails) override;
	void itemDropped(const SourceDetails &details) override;

	void showMenuAndAddItem(bool fromAddButton, Point<int> mousePos) override;

	BaseItemMinimalUI<DashboardItem> * createUIForItem(DashboardItem *) override;

	void newMessage(const ContainerAsyncEvent& e) override;
	void newMessage(const Parameter::ParameterEvent& e) override;
};