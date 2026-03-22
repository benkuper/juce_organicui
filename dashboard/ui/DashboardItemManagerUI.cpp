/*
  ==============================================================================

	DashboardItemManagerUI.cpp
	Created: 23 Apr 2017 12:33:58pm
	Author:  Ben

  ==============================================================================
*/

#include "JuceHeader.h"

using namespace juce;

std::function<void(PopupMenu*, int)> DashboardItemManagerUI::customAddItemsToMenuFunc = nullptr;
std::function<void(int, int, DashboardItemManagerUI*, Point<float>)> DashboardItemManagerUI::customHandleMenuResultFunc = nullptr;

DashboardItemManagerUI::DashboardItemManagerUI(DashboardItemManager* manager) :
	BaseManagerViewUI("Dashboard", manager)
{
	maxZoom = 2.0f;

	enableSnapping = DashboardManager::getInstance()->snapping->boolValue();
	updatePositionOnDragMove = true;


	manager->addAsyncCoalescedContainerListener(this);
	DashboardManager::getInstance()->snapping->addAsyncParameterListener(this);
	DashboardManager::getInstance()->editMode->addAsyncParameterListener(this);

	File f = manager->bgImage->getFile();
	if (f.existsAsFile()) bgImage = ImageCache::getFromFile(f);


	acceptedDropTypes.add("Controllable");
	acceptedDropTypes.add("Container");

	addExistingItems(false);

	setShowAddButton(false);
	setShowTools(DashboardManager::getInstance()->editMode->boolValue());
	tmpShowTools = showTools;
}

DashboardItemManagerUI::~DashboardItemManagerUI()
{
	if (!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
	if (DashboardManager::getInstanceWithoutCreating() != nullptr)
	{
		DashboardManager::getInstance()->snapping->removeAsyncParameterListener(this);
		DashboardManager::getInstance()->editMode->removeAsyncParameterListener(this);
	}
}

void DashboardItemManagerUI::resized()
{
	BaseManagerViewUI::resized();
	addItemBT->setBounds(juce::Rectangle<int>(0, 0, 50, 50));
}

void DashboardItemManagerUI::paint(Graphics& g)
{
	BaseManagerViewUI::paint(g);

	if (manager == nullptr || inspectable.wasObjectDeleted()) return;

	Point<float> p = manager->canvasSize->getPoint();
	juce::Rectangle<int> canvasR = getBoundsInView(juce::Rectangle<float>().withSizeKeepingCentre(p.x, p.y));
	g.setColour(manager->bgColor->getColor());
	g.fillRect(canvasR);

	float alpha = manager->bgImageAlpha->floatValue();

	if (bgImage.isNull() || alpha == 0) return;

	g.setColour(Colours::white.withAlpha(alpha));
	float scale = manager->bgImageScale->floatValue();

	juce::Rectangle<int> r = getBoundsInView(juce::Rectangle<float>().withSizeKeepingCentre(bgImage.getWidth() * scale, bgImage.getHeight() * scale));
	g.drawImage(bgImage, r.toFloat(), RectanglePlacement::stretchToFit, false);
}

void DashboardItemManagerUI::paintOverChildren(Graphics& g)
{
	BaseManagerViewUI::paintOverChildren(g);

	if (manager == nullptr || inspectable.wasObjectDeleted()) return;
	if (manager->canvasSize->enabled && manager->canvasSize->x > 0 && manager->canvasSize->y > 0)
	{
		Point<float> p = manager->canvasSize->getPoint();
		juce::Rectangle<int> canvasR = getBoundsInView(juce::Rectangle<float>().withSizeKeepingCentre(p.x, p.y));
		Path path;
		path.addRectangle(getLocalBounds());
		path.setUsingNonZeroWinding(false);
		path.addRectangle(canvasR);

		g.setColour(Colours::black.withAlpha(.5f));
		g.fillPath(path);

		int h = 50; // should be exposed
		juce::Rectangle<int> headerR = getBoundsInView(juce::Rectangle<float>().withSizeKeepingCentre(p.x, p.y).removeFromTop(h));;
		g.setColour(BLUE_COLOR.withAlpha(.1f));
		g.fillRect(headerR);
		float dashes[]{ 4, 3 };
		g.setColour(BLUE_COLOR.withAlpha(.3f));
		g.drawDashedLine(Line<float>(headerR.getBottomLeft().toFloat(), headerR.getBottomRight().toFloat()), dashes, 2, .5f);
	}
}


bool DashboardItemManagerUI::checkItemShouldBeVisible(BaseItemMinimalUI<DashboardItem>* ui)
{
	if (ui == nullptr) return false;
	if (ui->item == nullptr) return false;
	if (!ui->item->isVisible->boolValue()) return false;

	return BaseManagerViewUI::checkItemShouldBeVisible(ui);
}

bool DashboardItemManagerUI::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	return true;
}

void DashboardItemManagerUI::itemDropped(const SourceDetails& details)
{
	String type = details.description.getProperty("type", "").toString();

	BaseManagerViewUI::itemDropped(details);

	if (details.sourceComponent->getParentComponent() == this) return;

	InspectableContentComponent* icc = dynamic_cast<InspectableContentComponent*>(details.sourceComponent.get());

	if (icc != nullptr && icc->inspectable != nullptr)
	{
		BaseItem* bi = dynamic_cast<BaseItem*>(icc->inspectable.get());
		if (bi != nullptr)
		{
			if (bi == manager->parentContainer.get()) return;

			manager->selectItemWhenCreated = false; // remove auto selection to allow multi drag/drop flow
			manager->addItem(bi->createDashboardItem(), getViewMousePosition().toFloat());
			manager->selectItemWhenCreated = true;
		}
		return;
	}

	ControllableEditor* e = dynamic_cast<ControllableEditor*>(details.sourceComponent.get());
	if (e != nullptr)
	{
		manager->selectItemWhenCreated = false; // remove auto selection to allow multi drag/drop flow
		manager->addItem(e->controllable->createDashboardItem(), getViewMousePosition().toFloat());
		manager->selectItemWhenCreated = true;
		return;
	}

	GenericControllableContainerEditor* ge = dynamic_cast<GenericControllableContainerEditor*>(details.sourceComponent.get());
	if (ge != nullptr)
	{
		manager->selectItemWhenCreated = false; // remove auto selection to allow multi drag/drop flow
		manager->addItem(ge->container->createDashboardItem(), getViewMousePosition().toFloat());
		manager->selectItemWhenCreated = true;
		return;
	}
}

void DashboardItemManagerUI::showMenuAndAddItem(bool fromAddButton, Point<int> mousePos)
{
	if (!DashboardManager::getInstance()->editMode->boolValue()) return;
	//BaseManagerViewUI::showMenuAndAddItem(fromAddButton, mousePos);

	PopupMenu menu;
	menu.addItem(-1, "Add Group");
	menu.addItem(-2, "Add Comment");

#if ORGANICUI_USE_SHAREDTEXTURE
	menu.addItem(-3, "Add Shared Texture");
#endif

	menu.addItem(-4, "Add Dashboard Link");
	menu.addItem(-5, "Add IFrame");


	if (customAddItemsToMenuFunc) customAddItemsToMenuFunc(&menu, -5000);
	menu.addSeparator();

	manager->managerFactory->buildPopupMenu();
	menu.addSubMenu("Add Item", manager->managerFactory->getMenu());

	menu.addSeparator();
	menu.addItem(-10, showTools ? "Hide Tools" : "Show Tools");

	menu.showMenuAsync(PopupMenu::Options(), [this, mousePos](int result)
		{
			if (result == 0) return;



			Point<float> p = getViewPos(mousePos);
			if (result == -1) manager->addItem(new DashboardGroupItem(), p);
			else if (result == -2) manager->addItem(new DashboardCommentItem(), p);
#if ORGANICUI_USE_SHAREDTEXTURE
			else if (result == -3) manager->addItem(new SharedTextureDashboardItem(), p);
#endif
			else if (result == -4) manager->addItem(new DashboardLinkItem(), p);
			else if (result == -5) manager->addItem(new DashboardIFrameItem(), p);
			else if (result == -10) setShowTools(!showTools);
			else if (result < -1000) customHandleMenuResultFunc(result, -5000, this, p);
			else manager->addItem(manager->managerFactory->createFromMenuResult(result), p);


		}
	);
}

BaseItemMinimalUI<DashboardItem>* DashboardItemManagerUI::createUIForItem(DashboardItem* item)
{
	return item->createUI();
}

void DashboardItemManagerUI::newMessage(const ContainerAsyncEvent& e)
{
	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
		if (e.targetControllable.wasObjectDeleted()) return;

		if (e.targetControllable == manager->bgImage)
		{
			File f = manager->bgImage->getFile();
			if (f.existsAsFile()) bgImage = ImageCache::getFromFile(f);
			else bgImage = Image();
			repaint();
		}
		else if (e.targetControllable == manager->bgColor || e.targetControllable == manager->bgImageAlpha || e.targetControllable == manager->bgImageScale || e.targetControllable == manager->canvasSize)
		{
			repaint();
		}
		else if (DashboardItem* i = dynamic_cast<DashboardItem*>(e.targetControllable->parentContainer.get()))
		{
			if (e.targetControllable == i->isVisible)
			{
				updateItemVisibility(getUIForItem(i));
			}
		}

		break;
	default:
		break;
	}
}

void DashboardItemManagerUI::newMessage(const Parameter::ParameterEvent& e)
{
	if (e.parameter == DashboardManager::getInstance()->snapping)
	{
		enableSnapping = DashboardManager::getInstance()->snapping->boolValue();
	}
	else if (e.parameter == DashboardManager::getInstance()->editMode)
	{
		bool editMode = e.parameter->boolValue();
		if (!editMode) tmpShowTools = showTools;
		setShowTools(editMode ? tmpShowTools : false);
	}
}
