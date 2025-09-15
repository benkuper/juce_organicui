#include "JuceHeader.h"


BaseManagerUI::BaseManagerUI(const String& contentName, BaseManager* _manager, bool _useViewport) :
	InspectableContentComponent(_manager),
	baseManager(_manager),
	managerComparator(_manager),
	useViewport(_useViewport),
	defaultLayout(VERTICAL),
	container(this),
	headerSize(24),
	bgColor(BG_COLOR),
	labelHeight(10),
	minHeight(50),
	managerUIName(contentName),
	drawContour(false),
	transparentBG(false),
	resizeOnChildBoundsChanged(true),
	autoFilterHitTestOnItems(false),
	validateHitTestOnNoItem(true),
	showTools(false),
	animateItemOnAdd(true),
	useDedicatedSelector(true),
	selectingItems(false),
	isDraggingOver(false),
	highlightOnDragOver(true),
	fixedItemHeight(true),
	gap(2)
{

	selectionContourColor = LIGHTCONTOUR_COLOR;
	addItemText = "Add Item";

	if (useViewport)
	{
		container.addComponentListener(this);

		viewport.setViewedComponent(&container, false);
		viewport.setScrollBarsShown(true, false);
		viewport.setScrollOnDragMode(Viewport::ScrollOnDragMode::never);
		viewport.setScrollBarThickness(10);
		//viewport.setEnableKeyPressEvents(false);
		this->addAndMakeVisible(viewport);
	}

	addItemBT.reset(AssetManager::getInstance()->getAddBT());
	addItemBT->setWantsKeyboardFocus(false);

	addAndMakeVisible(addItemBT.get());
	addItemBT->addListener(this);

	setShowAddButton(baseManager->userCanAddItemsManually);

	acceptedDropTypes.add(baseManager->itemDataType);

	Engine::mainEngine->addAsyncEngineListener(this);

	InspectableSelectionManager::mainSelectionManager->addAsyncSelectionManagerListener(this);
	//must call addExistingItems from child class to get overrides

	setWantsKeyboardFocus(true);
}
