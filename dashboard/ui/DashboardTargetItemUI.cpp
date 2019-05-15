/*
  ==============================================================================

	DashboardTargetItemUI.cpp
	Created: 19 Apr 2017 11:01:13pm
	Author:  Ben

  ==============================================================================
*/


DashboardTargetItemUI::DashboardTargetItemUI(DashboardTargetItem * targetItem) :
	BaseItemUI(targetItem, Direction::ALL),
	targetItem(targetItem)
{
	setSize(0, 0);
	updateTargetUI();
}

DashboardTargetItemUI::~DashboardTargetItemUI()
{
}

void DashboardTargetItemUI::updateTargetUI()
{
	if (targetUI != nullptr)
	{
		removeChildComponent(targetUI.get());
	}

	DashboardItemTarget * t = targetItem->getDashboardTarget();
	if (t != nullptr) targetUI.reset(t->createDashboardContent());

	if (targetUI != nullptr)
	{
		addAndMakeVisible(targetUI.get());

		if (getWidth() == 0 || getHeight() == 0)
		{
			if (targetUI->getWidth() > 0) setContentSize(targetUI->getWidth(), targetUI->getHeight());
			else setContentSize(200, targetUI->getHeight());
		}
	}
}

void DashboardTargetItemUI::resizedInternalContent(juce::Rectangle<int>& r)
{
	if (targetUI != nullptr) targetUI->setBounds(r);
}

void DashboardTargetItemUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	BaseItemUI::controllableFeedbackUpdateInternal(c);
	if (c == targetItem->target) updateTargetUI();
}
