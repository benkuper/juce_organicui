/*
  ==============================================================================

	MovablePanel.cpp
	Created: 2 May 2016 3:08:37pm
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"

ShapeShifterPanel::ShapeShifterPanel(ShapeShifterContent* _content, ShapeShifterPanelTab* sourceTab) :
	ShapeShifter(ShapeShifter::PANEL),
	transparentBackground(false),
	targetMode(false),
	candidateZone(NONE),
	candidateTabIndex(-1),
	currentContent(nullptr)
{
	addAndMakeVisible(header);
	header.addHeaderListener(this);

	if (_content != nullptr)
	{
		if (sourceTab == nullptr)
		{
			addContent(_content);
		}
		else
		{
			attachTab(sourceTab);
		}
	}

}

ShapeShifterPanel::~ShapeShifterPanel()
{

	for (auto& c : contents) c->contentIsShown = false;
	header.clearTabs();
	header.removeHeaderListener(this);
	listeners.call(&Listener::panelDestroyed, this);
	masterReference.clear();
}


void ShapeShifterPanel::setCurrentContent(ShapeShifterContent* _content)
{
	if (_content == currentContent) return;

	if (currentContent != nullptr)
	{
		ShapeShifterPanelTab* tab = header.getTabForContent(currentContent);
		if (tab != nullptr) tab->setSelected(false);
		removeChildComponent(currentContent->contentComponent);
		currentContent->contentIsShown = false;
	}


	currentContent = _content;

	if (currentContent != nullptr)
	{
		ShapeShifterPanelTab* tab = header.getTabForContent(currentContent);
		if (tab != nullptr) tab->setSelected(true);

		currentContent->contentIsShown = true;
	}

	resized();

	if (currentContent != nullptr) addAndMakeVisible(currentContent->contentComponent); //add after resize
	//after resized
}

void ShapeShifterPanel::setCurrentContent(const String& name)
{
	ShapeShifterContent* c = getContentForName(name);
	if (c != nullptr) setCurrentContent(c);
}

void ShapeShifterPanel::setTargetMode(bool value)
{
	if (targetMode == value) return;
	targetMode = value;
	if (!targetMode)
	{
		candidateZone = NONE;
		candidateTabIndex = -1;
	}
	repaint();
}

void ShapeShifterPanel::paint(Graphics& g)
{
	auto bounds = getLocalBounds().toFloat().reduced(0.5f);
	g.setColour(BG_COLOR.withAlpha(transparentBackground ? .6f : 1.0f));
	g.fillRoundedRectangle(bounds, 6.0f);
	g.setColour(BG_COLOR.brighter(.1f).withAlpha(.4f));
	g.drawRoundedRectangle(bounds, 6.0f, 0.75f);
}

void ShapeShifterPanel::paintOverChildren(Graphics& g)
{
	if (!targetMode || candidateZone == NONE) return;

	auto preview = getLocalBounds().toFloat().reduced(4.0f);
	if (preview.isEmpty()) return;

	const char* label = "Add as tab";
	switch (candidateZone)
	{
	case LEFT:   preview = preview.removeFromLeft(preview.getWidth() * .5f); label = "Dock left"; break;
	case RIGHT:  preview = preview.removeFromRight(preview.getWidth() * .5f); label = "Dock right"; break;
	case TOP:    preview = preview.removeFromTop(preview.getHeight() * .5f); label = "Dock above"; break;
	case BOTTOM: preview = preview.removeFromBottom(preview.getHeight() * .5f); label = "Dock below"; break;
	case CENTER: break;
	case NONE:   return;
	}

	g.setColour(HIGHLIGHT_COLOR.withAlpha(.28f));
	g.fillRoundedRectangle(preview, 5.0f);
	g.setColour(HIGHLIGHT_COLOR.withAlpha(.9f));
	g.drawRoundedRectangle(preview.reduced(1.0f), 5.0f, 2.0f);
	g.setColour(TEXT_COLOR);
	g.drawFittedText(label, preview.toNearestInt().reduced(8), Justification::centred, 1);
	if (candidateZone == CENTER && candidateTabIndex >= 0)
	{
		const int x = candidateTabIndex < header.tabs.size()
			? header.tabs[candidateTabIndex]->getX()
			: (header.tabs.isEmpty() ? 3 : header.tabs.getLast()->getRight());
		g.setColour(HIGHLIGHT_COLOR);
		g.fillRoundedRectangle((float) (header.getX() + x - 2), (float) (header.getY() + 3),
			4.0f, (float) (header.getHeight() - 6), 2.0f);
	}
}

void ShapeShifterPanel::resized()
{
	juce::Rectangle<int> r = getLocalBounds().reduced(2);
	header.setBounds(r.removeFromTop(headerHeight));
	r.removeFromTop(1);
	if (currentContent != nullptr)
	{
		currentContent->contentComponent->setBounds(r);
	}
}

void ShapeShifterPanel::setTransparentBackground(bool value)
{
	if (transparentBackground == value) return;
	transparentBackground = value;
	repaint();
}

void ShapeShifterPanel::attachTab(ShapeShifterPanelTab* tab, int index)
{
	header.attachTab(tab, index);
	if (index < 0) contents.add(tab->content);
	else contents.insert(index, tab->content);
	setCurrentContent(tab->content);

}

void ShapeShifterPanel::detachTab(ShapeShifterPanelTab* tab, bool createNewPanel)
{
	ShapeShifterContent* content = tab->content;

	header.removeTab(tab, false);
	contents.removeObject(content, false);

	if (createNewPanel)
	{
		juce::Rectangle<int> r = getScreenBounds();
		ShapeShifterPanel* newPanel = ShapeShifterManager::getInstance()->createPanel(content, tab);
		ShapeShifterManager::getInstance()->showPanelWindow(newPanel, r);
	}

	if (currentContent == content)
	{
		if (contents.size() > 0)
		{
			setCurrentContent(contents[juce::jlimit<int>(0, contents.size() - 1, contents.indexOf(content))]);
		}
		else
		{
			DBG("panel emptied, num listeners " << listeners.size());
			listeners.call(&Listener::panelEmptied, this);
			ShapeShifterManager::getInstance()->removePanel(this);
		}
	}
}

void ShapeShifterPanel::addContent(ShapeShifterContent* content, bool setCurrent)
{
	header.addTab(content);
	contents.add(content);
	if (setCurrent) setCurrentContent(content);
}

bool ShapeShifterPanel::hasContent(ShapeShifterContent* content)
{
	for (auto& c : contents)
	{
		if (c == content) return true;
	}

	return false;
}


bool ShapeShifterPanel::hasContent(const String& name)
{
	return getContentForName(name) != nullptr;
}

ShapeShifterContent* ShapeShifterPanel::getContentForName(const String& name)
{
	for (auto& c : contents)
	{
		if (c == nullptr) continue;
		if (c->contentName == name) return c;
	}

	return nullptr;
}

bool ShapeShifterPanel::isFlexible()
{
	if (currentContent == nullptr) return false;
	return currentContent->contentIsFlexible;
}

void ShapeShifterPanel::removeTab(ShapeShifterPanelTab* tab)
{
	ShapeShifterContent* content = tab->content;
	contents.removeObject(content, false);
	header.removeTab(tab, true);
	if (currentContent == content)
	{
		if (contents.size() > 0)
		{
			setCurrentContent(contents[juce::jlimit<int>(0, contents.size() - 1, contents.indexOf(content) - 1)]);
		}
		else
		{
			listeners.call(&Listener::panelEmptied, this);
		}
	}

	listeners.call(&Listener::contentRemoved, content);
	delete content;

	if (contents.size() == 0)
		ShapeShifterManager::getInstance()->removePanel(this);
}

bool ShapeShifterPanel::attachPanel(ShapeShifterPanel* panel)
{
	ShapeShifterPanel* newPanelForTabs = nullptr;
	const int targetWidth = getWidth();
	const int targetHeight = getHeight();

	switch (candidateZone)
	{
	case NONE:
		return false;

	case LEFT:
	case RIGHT:
	case TOP:
	case BOTTOM:
		if (parentContainer == nullptr)
			return false;
		else
		{
			newPanelForTabs = ShapeShifterManager::getInstance()->createPanel(nullptr);
			parentContainer->insertPanelRelative(newPanelForTabs, this, candidateZone);
			if (candidateZone == LEFT || candidateZone == RIGHT)
			{
				setPreferredWidth(targetWidth / 2);
				newPanelForTabs->setPreferredWidth(targetWidth / 2);
			}
			else
			{
				setPreferredHeight(targetHeight / 2);
				newPanelForTabs->setPreferredHeight(targetHeight / 2);
			}
			newPanelForTabs->parentContainer->resized();
		}
		break;

	case CENTER:
	{
		newPanelForTabs = this;
		break;
	}
	}

	if (newPanelForTabs != nullptr)
	{
		int insertIndex = candidateTabIndex;
		int numTabs = panel->header.tabs.size();
		while (numTabs > 0)
		{
			ShapeShifterPanelTab* t = panel->header.tabs[0];
			panel->detachTab(t, false);
			newPanelForTabs->attachTab(t, insertIndex);
			if (insertIndex >= 0) ++insertIndex;
			numTabs--;
		}
	}
	return true;
}



ShapeShifterPanel::AttachZone ShapeShifterPanel::checkAttachZone(Point<int> screenPoint)
{
	AttachZone z = AttachZone::NONE;
	int tabIndex = -1;
	if (getWidth() > 0 && getHeight() > 0 && getScreenBounds().contains(screenPoint))
	{
		const auto point = getLocalPoint(nullptr, screenPoint);
		if (header.getBounds().contains(point))
		{
			z = CENTER;
			tabIndex = header.tabs.size();
			for (int i = 0; i < header.tabs.size(); ++i)
				if (point.x < header.getX() + header.tabs[i]->getBounds().getCentreX())
				{
					tabIndex = i;
					break;
				}
		}
		else
		{
			const float rx = (float) point.x / (float) getWidth();
			const float ry = (float) point.y / (float) getHeight();
			const float edge = jmin(jmin(rx, 1.0f - rx), jmin(ry, 1.0f - ry));
			if (edge >= .25f) z = CENTER;
			else if (edge == rx) z = LEFT;
			else if (edge == 1.0f - rx) z = RIGHT;
			else if (edge == ry) z = TOP;
			else z = BOTTOM;
		}
	}

	if (isDetached() && z != CENTER) z = NONE;
	if (candidateTabIndex != tabIndex)
	{
		candidateTabIndex = tabIndex;
		repaint();
	}

	setCandidateZone(z);
	return candidateZone;
}

void ShapeShifterPanel::setCandidateZone(AttachZone zone)
{
	if (candidateZone == zone) return;
	candidateZone = zone;
	repaint();
}


var ShapeShifterPanel::getCurrentLayout()
{
	var layout = ShapeShifter::getCurrentLayout();
	var tabData;
	for (auto& t : header.tabs)
	{
		if (t->content == nullptr) continue;
		var tData(new DynamicObject());
		tData.getDynamicObject()->setProperty("name", t->content->contentName);
		tabData.append(tData);
	}

	if (currentContent != nullptr) layout.getDynamicObject()->setProperty("currentContent", currentContent->contentName);
	layout.getDynamicObject()->setProperty("tabs", tabData);

	return layout;
}

void ShapeShifterPanel::loadLayoutInternal(var layout)
{
	juce::Array<var>* tabData = layout.getDynamicObject()->getProperty("tabs").getArray();

	if (tabData != nullptr)
	{
		for (auto& tData : *tabData)
		{
			String t = tData.getDynamicObject()->getProperty("name").toString();
			ShapeShifterContent* c = ShapeShifterFactory::createContent(tData.getDynamicObject()->getProperty("name"));
			if (c != nullptr) addContent(c);
		}
	}
	if (layout.getDynamicObject()->hasProperty("currentContent"))
	{
		setCurrentContent(layout.getDynamicObject()->getProperty("currentContent").toString());
	}
}

void ShapeShifterPanel::tabDrag(ShapeShifterPanelTab* tab, const MouseEvent& e)
{
	if (!isDetached() && contents.size() == 1)
	{
		headerDrag(e);
		return;
	}

	if (!isDetached() || contents.size() > 1)
	{
		ShapeShifterContent* draggedContent = tab->content;
		detachTab(tab, true);
		auto* manager = ShapeShifterManager::getInstance();
		if (auto* detachedPanel = manager->getPanelForContent(draggedContent))
			if (auto* window = manager->getWindowForPanel(detachedPanel))
				window->beginDrag(e, ShapeShifterWindow::TAB);
	}
}

void ShapeShifterPanel::tabReorder(ShapeShifterPanelTab* tab, int newIndex)
{
	const int oldIndex = header.tabs.indexOf(tab);
	if (oldIndex < 0 || oldIndex == newIndex) return;
	header.tabs.move(oldIndex, newIndex);
	contents.move(oldIndex, newIndex);
	header.resized();
}

void ShapeShifterPanel::tabSelect(ShapeShifterPanelTab* tab)
{
	setCurrentContent(tab->content);
}

void ShapeShifterPanel::askForRemoveTab(ShapeShifterPanelTab* tab)
{
	removeTab(tab);
}

void ShapeShifterPanel::headerDrag(const MouseEvent& e)
{
	if (!isDetached())
	{
		listeners.call(&Listener::panelDetach, this);
		if (isDetached())
			if (auto* window = ShapeShifterManager::getInstance()->getWindowForPanel(this))
				window->beginDrag(e, ShapeShifterWindow::PANEL);
	}
}
