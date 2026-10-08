/*
  ==============================================================================

    ShapeShifterPanelHeader.cpp
    Created: 3 May 2016 2:25:55pm
    Author:  bkupe

  ==============================================================================
*/

ShapeShifterPanelHeader::ShapeShifterPanelHeader()
{
	addMouseListener(this, true);
}

ShapeShifterPanelHeader::~ShapeShifterPanelHeader()
{

}

void ShapeShifterPanelHeader::addTab(ShapeShifterContent * content)
{
	ShapeShifterPanelTab * tab = new ShapeShifterPanelTab(content);
	attachTab(tab);
}

void ShapeShifterPanelHeader::removeTab(ShapeShifterPanelTab * tab, bool doRemove)
{
	tab->removeShapeShifterTabListener(this);
	removeChildComponent(tab);
	tabs.removeObject(tab,doRemove);
	resized();
}

void ShapeShifterPanelHeader::attachTab(ShapeShifterPanelTab * tab, int index)
{
	tab->addShapeShifterTabListener(this);
	addAndMakeVisible(tab);
	if (index < 0) tabs.add(tab);
	else tabs.insert(index, tab);
	resized();
}

void ShapeShifterPanelHeader::clearTabs()
{
	for (auto& t : tabs)
	{ 
		t->removeShapeShifterTabListener(this);
		t->clear();
		removeChildComponent(t);
	}
	tabs.clear();
}

ShapeShifterPanelTab * ShapeShifterPanelHeader::getTabForContent(ShapeShifterContent * content)
{
	for (auto &t : tabs)
	{
		if (t->content == content) return t;
	}
	return nullptr;
}

void ShapeShifterPanelHeader::mouseDown(const MouseEvent & e)
{
	ShapeShifterPanelTab * tab = dynamic_cast<ShapeShifterPanelTab *>(e.eventComponent);
	if (tab != nullptr)
	{
		listeners.call(&Listener::tabSelect, tab);
	}

}

void ShapeShifterPanelHeader::mouseDrag(const MouseEvent & e)
{
	if (ShapeShifterManager::getInstance()->lockMode) return;

	if (e.eventComponent == this)
	{
		if (e.getDistanceFromDragStart() >= 8) listeners.call(&Listener::headerDrag, e);
	}else
	{
		ShapeShifterPanelTab * tab = dynamic_cast<ShapeShifterPanelTab *>(e.eventComponent);
		if (tab == nullptr) return;

		if (std::abs(e.getDistanceFromDragStartY()) < 12)
		{
			if (std::abs(e.getDistanceFromDragStartX()) < 8) return;
			const float x = e.getEventRelativeTo(this).position.x;
			int newIndex = tabs.indexOf(tab);
			while (newIndex > 0 && x < tabs[newIndex - 1]->getBounds().getCentreX()) --newIndex;
			while (newIndex < tabs.size() - 1 && x > tabs[newIndex + 1]->getBounds().getCentreX()) ++newIndex;
			if (newIndex != tabs.indexOf(tab)) listeners.call(&Listener::tabReorder, tab, newIndex);
		}
		else listeners.call(&Listener::tabDrag, tab, e);
	}

}

void ShapeShifterPanelHeader::paint(Graphics & g)
{
	g.setColour(BG_COLOR.brighter(.1f));
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
}

void ShapeShifterPanelHeader::resized()
{
	//re arrange tabs
	juce::Rectangle<int> r = getLocalBounds();
	if (r.getWidth() == 0 || r.getHeight() == 0) return;

	for (auto &t : tabs)
	{
		t->setBounds(r.removeFromLeft(jmin<int>(r.getWidth(), t->getLabelWidth() + 8)).reduced(2, 2));
	}
}

void ShapeShifterPanelHeader::askForRemoveTab(ShapeShifterPanelTab * tab)
{
	listeners.call(&Listener::askForRemoveTab, tab);
}

void ShapeShifterPanelHeader::nameChanged(ShapeShifterPanelTab*)
{
	resized();
}
