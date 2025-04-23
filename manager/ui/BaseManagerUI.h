/*
  ==============================================================================

	ManagerUI.h
	Created: 28 Oct 2016 8:03:45pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once
#pragma warning(disable:4244)



class ManagerUIItemComparator
{
public:
	ManagerUIItemComparator(BaseManager* _manager) :manager(_manager) {}

	BaseManager* manager;

	int compareElements(BaseItemMinimalUI* u1, BaseItemMinimalUI* u2)
	{
		return (manager->baseItems.indexOf(u1->baseItem) < manager->baseItems.indexOf(u2->baseItem)) ? -1 : 1;
	}
};

class BaseManagerUI;

class BaseManagerUIItemContainer :
	public juce::Component
{
public:
	BaseManagerUIItemContainer(BaseManagerUI* mui) : baseManagerUI(mui) {};
	~BaseManagerUIItemContainer() {}

	BaseManagerUI* baseManagerUI;

	void childBoundsChanged(juce::Component* c);
};



class BaseManagerUI :
	public InspectableContentComponent,
	//public BaseManager::ManagerListener,
	//public BaseManager::AsyncListener,
	public juce::Button::Listener,
	public BaseItemUI::ItemUIListener,
	public BaseItemMinimalUI::ItemMinimalUIListener,
	public InspectableSelectionManager::AsyncListener,
	public juce::ComponentListener,
	public Engine::AsyncListener,
	public juce::DragAndDropTarget,
	public juce::TextEditor::Listener
{
public:
	BaseManagerUI(const juce::String& contentName, BaseManager* _manager, bool _useViewport = true);
	virtual ~BaseManagerUI();

	enum Layout { HORIZONTAL, VERTICAL, FREE };

	BaseManager* baseManager;
	juce::OwnedArray<BaseItemMinimalUI> baseItemsUI;
	ManagerUIItemComparator managerComparator;

	BaseManagerUIItemContainer container;

	//ui
	bool useViewport; //TODO, create a ManagerViewportUI

	Layout defaultLayout;

	class ManagerViewport :
		public juce::Viewport
	{
	public:
		bool useMouseWheelMoveIfNeeded(const juce::MouseEvent& e, const juce::MouseWheelDetails& d) override
		{
			if (e.mods.isShiftDown()) return false;
			return juce::Viewport::useMouseWheelMoveIfNeeded(e, d);
		}
	};
	ManagerViewport viewport;

	int headerSize;
	juce::Colour bgColor;
	int labelHeight;
	int minHeight;
	juce::String managerUIName;
	bool drawContour;
	bool transparentBG;
	bool resizeOnChildBoundsChanged;
	bool autoFilterHitTestOnItems;
	bool validateHitTestOnNoItem;

	std::unique_ptr<juce::ImageButton> addItemBT;
	std::unique_ptr<juce::TextEditor> searchBar;

	class ToolContainer : public juce::Component
	{
	public:
		ToolContainer() : juce::Component("Tools") {}
		void paint(juce::Graphics& g) override { g.setColour(BG_COLOR.darker(.6f)); g.fillRoundedRectangle(getLocalBounds().toFloat(), 2); }
	};
	ToolContainer toolContainer;
	juce::OwnedArray<juce::Component> tools;
	juce::HashMap<juce::Button*, std::function<void()>> toolFuncMap;
	bool showTools;

	//ui
	juce::String noItemText;

	//menu
	juce::String addItemText;

	//Animation
	bool animateItemOnAdd;
	juce::ComponentAnimator itemAnimator;

	//selection
	bool useDedicatedSelector;
	bool selectingItems;

	//drag and drop
	juce::StringArray acceptedDropTypes;
	bool isDraggingOver;
	bool highlightOnDragOver;
	int currentDropIndex;

	//layout
	bool fixedItemHeight;
	int gap;

	void setDefaultLayout(Layout l);
	void addExistingItems(bool resizeAfter = true);

	void setShowAddButton(bool value);
	void setShowSearchBar(bool value);

	//tools
	void setShowTools(bool value);
	void addButtonTool(juce::Button* c, std::function<void()> clickFunc = nullptr);
	void addControllableTool(ControllableUI* c);

	virtual void paint(juce::Graphics& g) override;

	virtual void resized() override;
	virtual juce::Rectangle<int> setHeaderBounds(juce::Rectangle<int>& r);
	virtual void resizedInternalHeader(juce::Rectangle<int>& r);
	virtual void resizedInternalHeaderTools(juce::Rectangle<int>& r);
	virtual void resizedInternalContent(juce::Rectangle<int>& r);
	virtual void placeItems(juce::Rectangle<int>& r);
	virtual void resizedInternalFooter(juce::Rectangle<int>& r);

	enum AlignMode { LEFT, RIGHT, TOP, BOTTOM, CENTER_H, CENTER_V };
	virtual void alignItems(AlignMode alignMode);

	class PositionComparator
	{
	public:
		PositionComparator(bool isVertical) : isVertical(isVertical) {}
		bool isVertical;
		int compareElements(BaseItem* i1, BaseItem* i2) {
			return isVertical ? i2->viewUIPosition->y - i1->viewUIPosition->y : i2->viewUIPosition->x - i1->viewUIPosition->x;
		}
	};
	virtual void distributeItems(bool isVertical);

	virtual void updateItemsVisibility();
	virtual void updateItemVisibilityManagerInternal(BaseItemMinimalUI* bui);
	virtual void updateBaseItemVisibility(BaseItemMinimalUI* bui);

	virtual bool hasFiltering();
	virtual juce::Array<BaseItemMinimalUI*> getFilteredItems();
	virtual bool checkFilterForItem(BaseItemMinimalUI* item);

	virtual void childBoundsChanged(juce::Component*) override;

	virtual bool hitTest(int x, int y) override;

	//For container check
	virtual void componentMovedOrResized(juce::Component& c, bool wasMoved, bool wasResized) override;


	virtual void showMenuAndAddItem(bool isFromAddButton, juce::Point<int> mouseDownPos);
	virtual void showMenuAndAddItem(bool isFromAddButton, juce::Point<int> mouseDownPos, std::function<void(BaseItem*)> callback);
	virtual void addMenuExtraItems(juce::PopupMenu& p, int startIndex) {}
	virtual void handleMenuExtraItemsResult(int result, int startIndex) {}
	virtual void addItemFromMenu(bool isFromAddButton, juce::Point<int> mouseDownPos);
	virtual void addBaseItemFromMenu(BaseItem* item, bool isFromAddButton, juce::Point<int> mouseDownPos);

	virtual BaseItemMinimalUI* addItemUI(BaseItem* item, bool animate = false, bool resizeAndRepaint = true);
	virtual BaseItemMinimalUI* createBaseUIForItem(BaseItem* item);
	virtual void addItemUIManagerInternal(BaseItemMinimalUI*) {}

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseUp(const juce::MouseEvent& e) override;

	virtual void askSelectToThis(BaseItemMinimalUI* item) override;

	virtual void removeItemUI(BaseItem* item, bool resizeAndRepaint = true);
	virtual void removeItemUIManagerInternal(BaseItemMinimalUI*) {}

	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragMove(const SourceDetails& dragSourceDetails) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails& dragSourceDetails) override;

	//Selection
	virtual void addSelectableComponentsAndInspectables(juce::Array<juce::Component*>& selectables, juce::Array<Inspectable*>& inspectables);
	virtual juce::Component* getSelectableComponentForBaseItemUI(BaseItemMinimalUI* itemUI);

	virtual int getDropIndexForPosition(juce::Point<int> localPosition);
	virtual void itemUIMiniModeChanged(BaseItemUI* se) override {}

	//menu
	BaseItemMinimalUI* getBaseUIForItem(BaseItem* item, bool directIndexAccess = true);

	int getContentHeight();

	virtual void itemAddedAsync(BaseItem* item);
	virtual void itemsAddedAsync(juce::Array<BaseItem*> items);

	virtual void groupAddedAsync(BaseItem *item);
	virtual void groupsAddedAsync(juce::Array<BaseItem*> items);

	virtual void itemsReorderedAsync();


	virtual void newMessage(const InspectableSelectionManager::SelectionEvent& e) override;

	void buttonClicked(juce::Button* b) override;
	void textEditorTextChanged(juce::TextEditor& e) override;
	void textEditorReturnKeyPressed(juce::TextEditor& e) override;

	virtual void inspectableDestroyed(Inspectable*) override;

	virtual void newMessage(const Engine::EngineEvent& e) override;

	virtual void notifyItemUIAdded(BaseItemMinimalUI* itemUI) {}
	virtual void notifyItemUIRemoved(BaseItemMinimalUI* itemUI) {}
};

