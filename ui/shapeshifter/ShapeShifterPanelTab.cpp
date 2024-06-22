#include "ShapeShifterPanelTab.h"
/*
  ==============================================================================

	ShapeShifterPanelTab.cpp
	Created: 3 May 2016 12:43:01pm
	Author:  bkupe

  ==============================================================================
*/

ShapeShifterPanelTab::ShapeShifterPanelTab(ShapeShifterContent* _content) : content(_content), selected(false)
{
	panelLabel.setInterceptsMouseClicks(false, false);

	panelLabel.setFont(FontOptions(GlobalSettings::getInstance()->fontSize->floatValue()));
	panelLabel.setColour(panelLabel.textColourId, TEXT_COLOR);
	panelLabel.setJustificationType(Justification::centred);
	jassert(content != nullptr);
	String n = content->customName.isNotEmpty() ? content->customName : content->contentName;
	panelLabel.setText(content == nullptr ? "[No content]" : n, NotificationType::dontSendNotification);

	addAndMakeVisible(&panelLabel);

	closePanelBT.reset(AssetManager::getInstance()->getRemoveBT());
	closePanelBT->addListener(this);
	closePanelBT->setWantsKeyboardFocus(false);

	addAndMakeVisible(closePanelBT.get());

	setSize(getLabelWidth(), 20);

	if (content != nullptr) content->addAsyncShapeShifterContentListener(this);
}

ShapeShifterPanelTab::~ShapeShifterPanelTab()
{
	if (content != nullptr) content->removeAsyncShapeShifterContentListener(this);
}

void ShapeShifterPanelTab::setSelected(bool value)
{
	selected = value;
	repaint();
}

void ShapeShifterPanelTab::clear()
{
	if (content != nullptr)
	{
		content->removeAsyncShapeShifterContentListener(this);
		content = nullptr;
	}
}

void ShapeShifterPanelTab::paint(Graphics& g)
{
	g.setColour(selected ? BG_COLOR : BG_COLOR.brighter(.15f));
	juce::Rectangle<int> r = getLocalBounds();
	if (!selected) r.reduce(1, 1);
	g.fillRect(r);
}

void ShapeShifterPanelTab::resized()
{
	juce::Rectangle<int> r = getLocalBounds();
	closePanelBT->setBounds(r.removeFromRight(r.getHeight()).reduced(3));
	panelLabel.setBounds(r);
}

int ShapeShifterPanelTab::getLabelWidth()
{
	return panelLabel.getFont().getStringWidth(panelLabel.getText()) + 30;
}

void ShapeShifterPanelTab::buttonClicked(Button* b)
{
	if (b == closePanelBT.get()) tabListeners.call(&TabListener::askForRemoveTab, this);
}

void ShapeShifterPanelTab::mouseDoubleClick(const MouseEvent& e)
{
	ShapeShifterManager::getInstance()->toggleTemporaryFullContent(content);
}

void ShapeShifterPanelTab::newMessage(const ShapeShifterContent::ShapeShifterContentEvent& e)
{
	switch (e.type)
	{
	case ShapeShifterContent::ShapeShifterContentEvent::NAME_CHANGED:
		panelLabel.setText(content->customName.isNotEmpty() ? content->customName : content->contentName, NotificationType::dontSendNotification);
		setSize(getLabelWidth(), 20);
		tabListeners.call(&TabListener::nameChanged, this);
		break;
	}
}
