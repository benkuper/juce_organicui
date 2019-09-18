/*
  ==============================================================================

    ShapeShifterPanelTab.cpp
    Created: 3 May 2016 12:43:01pm
    Author:  bkupe

  ==============================================================================
*/

ShapeShifterPanelTab::ShapeShifterPanelTab(ShapeShifterContent * _content) : content(_content), selected(false)
{
	panelLabel.setInterceptsMouseClicks(false, false);

	panelLabel.setFont(14);
	panelLabel.setColour(panelLabel.textColourId, TEXT_COLOR);
	panelLabel.setJustificationType(Justification::centred);
	jassert(content != nullptr);
	panelLabel.setText(content == nullptr?"[No content]":content->contentName, NotificationType::dontSendNotification);

	addAndMakeVisible(&panelLabel);

	closePanelBT.reset(AssetManager::getInstance()->getRemoveBT());
	closePanelBT->addListener(this);
	addAndMakeVisible(closePanelBT.get());

	setSize(getLabelWidth(), 20);
}

ShapeShifterPanelTab::~ShapeShifterPanelTab()
{

}

void ShapeShifterPanelTab::setSelected(bool value)
{
	selected = value;
	repaint();
}

void ShapeShifterPanelTab::paint(Graphics & g)
{
	g.setColour(selected?BG_COLOR:BG_COLOR.brighter(.15f));
 juce::Rectangle<int> r = getLocalBounds();
	if (!selected) r.reduce(1,1);
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

void ShapeShifterPanelTab::buttonClicked(Button * b)
{
	if(b == closePanelBT.get()) tabListeners.call(&TabListener::askForRemoveTab, this);
}
