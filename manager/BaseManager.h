/*
  ==============================================================================

	Manager.h
	Created: 28 Oct 2016 8:03:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class BaseItemGroup;

class BaseManager :
	public EnablingControllableContainer,
	public BaseItemListener
{
public:
	BaseManager(const juce::String& name, bool canHaveGroups = false);
	virtual ~BaseManager();

	bool canHaveGroups;
	bool userCanAddItemsManually;
	bool selectItemWhenCreated;
	bool isManipulatingMultipleItems;
	juce::Point<float> clipboardCopyOffset;

	//ui
	juce::Point<int> viewOffset; //in pixels, viewOffset of 0 means zeroPos is at the center of the window
	float viewZoom;
	bool showItemsInEditor;
	BoolParameter* snapGridMode;
	BoolParameter* showSnapGrid;
	IntParameter* snapGridSize;

	//Factory and items
	juce::OwnedArray<BaseItem, juce::CriticalSection> baseItems;
	juce::String itemDataType;


	virtual void clear() override;

	//Creating and adding items
	virtual BaseItem* createItem() { jassertfalse; return nullptr; }
	virtual BaseItem* addItem(BaseItem* item, juce::var data = juce::var(), bool addToUndo = true, bool notify = true);
	juce::Array<BaseItem*> addItems(juce::Array<BaseItem*> items, juce::var data = juce::var(), bool addToUndo = true, bool notify = true);

	virtual BaseItem* createItemFromData(juce::var data) = 0;
	virtual BaseItem* addItemFromData(juce::var data, bool addToUndo = true);
	virtual juce::Array<BaseItem*> addItemsFromData(juce::var data, bool addToUndo = true);
	virtual juce::Array<BaseItem*> addItemsFromClipboard(bool showWarning = true);

	virtual bool canAddItemOfType(const juce::String& typeToCheck);
	virtual void loadItemsData(juce::var data);

	//remove
	BaseItem* removeItem(BaseItem* item, bool addToUndo = true, bool notify = true, bool returnItem = false);
	void removeItems(juce::Array<BaseItem*> items, bool addToUndo = true, bool notify = true);

	//Undo
	virtual juce::UndoableAction* getAddBaseItemUndoableAction(BaseItem* item = nullptr, juce::var data = juce::var());
	virtual juce::UndoableAction* getAddBaseItemsUndoableAction(juce::Array<BaseItem*> item = nullptr, juce::var data = juce::var());
	virtual juce::Array<juce::UndoableAction*> getRemoveBaseItemUndoableAction(BaseItem* item);
	virtual juce::Array<juce::UndoableAction*> getRemoveBaseItemsUndoableAction(juce::Array<BaseItem*> items);

	//Ordering
	virtual void reorderItems() {}
	virtual void setItemIndex(BaseItem* item, int newIndex, bool addToUndo = true);
	virtual juce::Array<juce::UndoableAction*> getSetItemIndexUndoableAction(BaseItem* item, int newIndex);


	//Listing items
	virtual BaseItem* getItemWithName(const juce::String& itemShortName, bool searchNiceNameToo = false, bool searchWithLowerCaseIfNotFound = true);

	virtual juce::String getItemPath(BaseItem* item);
	virtual BaseItem* getItemWithPath(const juce::String& relativePath);

	template<class IType>
	juce::Array<IType*> getItemsWithType(bool recursive = true)
	{
		juce::Array<BaseItem*> itemsToSearch = getBaseItems(recursive, true, false);
		juce::Array<IType*> result;
		for (auto& i : itemsToSearch)
		{
			if (IType* it = dynamic_cast<IType*>(i)) result.add(it);
		}

		return result;
	}

	juce::Array<BaseItem*> getBaseItems(bool recursive = true, bool includeDisabled = true, bool includeGroups = true) const;
	void callFunctionOnAllItems(bool recursive, bool includeGroups, bool includeDisabled, std::function<void(BaseItem*)> func);

	virtual void handleAddFromRemoteControl(juce::var data) override;

	void askForPaste() override;
	void askForSelectAllItems(bool addToSelection = false) override;

	virtual juce::var getJSONData(bool includeNonOverriden = false) override;

	virtual juce::var getExportSelectionData();
	virtual juce::var getRemoteControlData() override;
	juce::String getScriptTargetString() override;

	class BaseManagerEvent
	{
	public:
		enum Type {
			ITEM_ADDED,
			ITEM_REMOVED,
			ITEMS_ADDED,
			ITEMS_REMOVED,
			ITEMS_REORDERED,
			MANAGER_CLEARED,
			NEEDS_UI_UPDATE
		};

		BaseManagerEvent(Type t, juce::Array<BaseItem*> iList, bool fromChildGroup = false);
		virtual ~BaseManagerEvent() {}

		Type type;
		juce::Array<juce::WeakReference<BaseItem>> baseItems;
		bool fromChildGroup;
	};

	//UNDO MANAGER
	class ManagerBaseAction :
		public juce::UndoableAction
	{
	public:
		ManagerBaseAction(BaseManager* manager, juce::var _data = juce::var());

		juce::String managerControlAddress;
		juce::var data;
		juce::WeakReference<Inspectable> managerRef;

		BaseManager* getManager();
	};

	class ItemBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemBaseAction(BaseManager* m, BaseItem* i, juce::var data = juce::var());

		juce::WeakReference<BaseItem> baseItem;
		juce::String itemShortName;
		int itemIndex;

		BaseItem* getItem();
	};

	class AddItemAction :
		public ItemBaseAction
	{
	public:
		AddItemAction(BaseManager* m, BaseItem* i, juce::var data = juce::var());

		bool perform() override;
		bool undo() override;
	};

	class RemoveItemAction :
		public ItemBaseAction
	{
	public:
		RemoveItemAction(BaseManager* m, BaseItem* i, juce::var data = juce::var());

		bool perform() override;
		bool undo() override;
	};

	class MoveItemAction :
		public ItemBaseAction
	{
	public:
		MoveItemAction(BaseManager* m, BaseItem* i, int prevIndex, int newIndex);

		int prevIndex;
		int newIndex;

		bool perform() override;
		bool undo() override;
	};

	//Multi add/remove items actions
	class ItemsBaseAction :
		public ManagerBaseAction
	{
	public:
		ItemsBaseAction(BaseManager* m, juce::Array<BaseItem*> iList, juce::var data = juce::var());

		juce::Array<juce::WeakReference<BaseItem>> baseItems;
		juce::StringArray itemsShortName;

		juce::Array<BaseItem*> getItems();

	};

	class AddItemsAction :
		public ItemsBaseAction
	{
	public:
		AddItemsAction(BaseManager* m, juce::Array<BaseItem*> iList, juce::var data = juce::var());

		int startIndex;
		bool perform() override;
		bool undo() override;
	};


	class RemoveItemsAction :
		public ItemsBaseAction
	{
	public:
		RemoveItemsAction(BaseManager* m, juce::Array<BaseItem*> iList);

		bool perform() override;
		bool undo() override;
	};

protected:
	void askForRemoveBaseItem(BaseItem* item) override;
	void askForDuplicateItem(BaseItem* item) override;
	void askForMoveBefore(BaseItem*) override;
	void askForMoveAfter(BaseItem*) override;
	void askForSelectPreviousItem(BaseItem* item, bool addToSelection = false) override;
	void askForSelectNextItem(BaseItem* item, bool addToSelection = false) override;

	void onContainerParameterChanged(Parameter* p) override;

	virtual void loadJSONDataInternal(juce::var data) override;
	virtual void loadJSONDataManagerInternal(juce::var data);

	virtual void getRemoteControlDataInternal(juce::var& data) override;

	virtual void addItemManagerInternal(BaseItem* item, juce::var data) {}
	virtual void addItemsManagerInternal(juce::Array<BaseItem*> items, juce::var data) {}
	virtual void removeItemManagerInternal(BaseItem* item) {}
	virtual void removeItemsManagerInternal(juce::Array<BaseItem*> items) {}

	//notify, to override by templated classes
	virtual void notifyItemAdded(BaseItem* item, bool fromChildGroup = false) {}
	virtual void notifyItemsAdded(juce::Array<BaseItem*> items, bool fromChildGroup = false) {}
	virtual void notifyItemRemoved(BaseItem* item, bool fromChildGroup = false) {}
	virtual void notifyItemsRemoved(juce::Array<BaseItem*> items, bool fromChildGroup = false) {}
	virtual void notifyItemsReordered(bool fromChildGroup = false) {}

	virtual void notifyAsync(BaseManagerEvent::Type type, juce::Array<BaseItem*> items = juce::Array<BaseItem*>(), bool fromChildGroup = false) {}
};

