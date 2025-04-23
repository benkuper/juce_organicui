/*
  ==============================================================================

	ManagerUI.h
	Created: 28 Oct 2016 8:03:45pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once
#pragma warning(disable:4244)


template<class T> class ItemMinimalUI;
template<class T> class ItemBaseGroup;
template<class T> class ItemGroup;

template<class M, class T, class U = ItemUI<T>, class G = ItemBaseGroup<T>, class GU= ItemGroupUI<T>>
class ManagerUI :
	public BaseManagerUI,
	public Manager<T>::ManagerListener,
	public Manager<T>::AsyncListener
{
public:

	static_assert(std::is_base_of<Manager<T, G>, M>::value, "M must be derived from Manager<T>");
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	static_assert(std::is_base_of<BaseItemMinimalUI, U>::value, "U must be derived from BaseItemMinimalUI");
	static_assert(std::is_base_of<ItemGroup<T>, G>::value, "G must be derived from ItemBaseGroup<T>");
	static_assert(std::is_base_of<BaseItemMinimalUI, GU>::value, "GU must be derived from BaseItemMinimalUI");

	ManagerUI(const juce::String& contentName, M* _manager, bool _useViewport = true);
	virtual ~ManagerUI();

	M* manager;
	juce::Array<U*> itemsUI;
	juce::Array<GU*> groupsUI;

	BaseItemMinimalUI* createBaseUIForItem(BaseItem* item) override;
	virtual U* createUIForItem(T* item);
	virtual GU* createUIForGroup(G* item);

	virtual void addItemUIManagerInternal(BaseItemMinimalUI* itemUI) override;
	virtual void addItemUIInternal(U* itemUI) {}

	virtual void removeItemUIManagerInternal(BaseItemMinimalUI* itemUI) override;
	virtual void removeItemUIInternal(U* itemUI) {}

	virtual void addBaseItemFromMenu(BaseItem* item, bool isFromAddButton, juce::Point<int> mouseDownPos) override;
	virtual void addItemFromMenu(T* item, bool isFromAddButton, juce::Point<int> mouseDownPos);

	virtual void updateItemVisibilityManagerInternal(BaseItemMinimalUI* bui) override;
	virtual void updateItemVisibility(U* ui);

	U* getUIForItem(T* item, bool directIndexAccess = true);

	virtual void showMenuAndAddItem(bool isFromAddButton, juce::Point<int> mouseDownPos, std::function<void(BaseItem*)> callback) override;

	virtual juce::Component* getSelectableComponentForBaseItemUI(BaseItemMinimalUI* itemUI) override;
	virtual juce::Component* getSelectableComponentForItemUI(U* itemUI);

	virtual void itemRemoved(T* item) override; //must keep this one realtime because the async may cause the target item to already be deleted by the time this function is called
	virtual void itemsRemoved(juce::Array<T*> items) override;
	virtual void groupRemoved(G* group) override;
	virtual void groupsRemoved(juce::Array<G*> items) override;

	void newMessage(const typename Manager<T>::ManagerEvent& e) override;


	virtual void inspectableDestroyed(Inspectable*) override;

	virtual void notifyItemUIAdded(BaseItemMinimalUI* ui) override;
	virtual void notifyItemUIRemoved(BaseItemMinimalUI* ui) override;

	class  ManagerUIListener
	{
	public:
		/** Destructor. */
		virtual ~ManagerUIListener() {}
		virtual void itemUIAdded(U*) {}
		virtual void itemUIRemoved(U*) {}
	};

	juce::ListenerList<ManagerUIListener> managerUIListeners;
	void addManagerUIListener(ManagerUIListener* newListener) { managerUIListeners.add(newListener); }
	void removeManagerUIListener(ManagerUIListener* listener) { managerUIListeners.remove(listener); }
};


template<class M, class T, class U, class G, class GU>
ManagerUI<M, T, U, G, GU>::ManagerUI(const juce::String& contentName, M* _manager, bool _useViewport) :
	BaseManagerUI(contentName, _manager, _useViewport),
	manager(_manager)
{
	manager->addManagerListener(this);
	manager->addAsyncManagerListener(this);
}


template<class M, class T, class U, class G, class GU>
ManagerUI<M, T, U, G, GU>::~ManagerUI()
{
	if (!inspectable.wasObjectDeleted())
	{
		this->manager->removeManagerListener(this);
		this->manager->removeAsyncManagerListener(this);
	}

	Engine::mainEngine->removeAsyncEngineListener(this);
	if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeAsyncSelectionManagerListener(this);

}

template<class M, class T, class U, class G, class GU>
BaseItemMinimalUI* ManagerUI<M, T, U, G, GU>::createBaseUIForItem(BaseItem* item)
{
	if (item->isGroup) return createUIForGroup((G*)item);
	return this->createUIForItem((T*)item);
}

template<class M, class T, class U, class G, class GU>
U* ManagerUI<M, T, U, G, GU>::createUIForItem(T* item)
{
	return new U(item);
}

template<class M, class T, class U, class G, class GU>
GU* ManagerUI<M, T, U, G, GU>::createUIForGroup(G* item)
{
	return new GU(item);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::addItemUIManagerInternal(BaseItemMinimalUI* itemUI)
{
	if (itemUI->isGroupUI()) itemsUI.add((U*)itemUI);
	else groupsUI.add((GU*)itemUI);

	addItemUIInternal((U*)itemUI);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::removeItemUIManagerInternal(BaseItemMinimalUI* itemUI)
{
	if (itemUI->isGroupUI()) itemsUI.removeAllInstancesOf((U*)itemUI);
	else groupsUI.removeAllInstancesOf((GU*)itemUI);
	removeItemUIInternal((U*)itemUI);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::addBaseItemFromMenu(BaseItem* item, bool isFromAddButton, juce::Point<int> mouseDownPos)
{
	return addItemFromMenu((T*)item, isFromAddButton, mouseDownPos);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::addItemFromMenu(T* item, bool isFromAddButton, juce::Point<int> mouseDownPos)
{
	this->manager->addItem(item);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::updateItemVisibilityManagerInternal(BaseItemMinimalUI* ui) {
	this->updateItemVisibility((U*)ui);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::updateItemVisibility(U* ui)
{
	this->updateBaseItemVisibility(ui);
}

template<class M, class T, class U, class G, class GU>
U* ManagerUI<M, T, U, G, GU>::getUIForItem(T* item, bool directIndexAccess)
{
	return (U*)getBaseUIForItem(item, directIndexAccess);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::showMenuAndAddItem(bool isFromAddButton, juce::Point<int> mouseDownPos, std::function<void(BaseItem*)> callback)
{
	if (manager->managerFactory != nullptr)
	{
		manager->managerFactory->showCreateMenu(manager, [this, isFromAddButton, mouseDownPos, callback](T* item)
			{
				this->addItemFromMenu(item, isFromAddButton, mouseDownPos);
				if (callback != nullptr) callback(item);

			}
		);
		return;
	}

	if (isFromAddButton)
	{
		T* item = manager->Manager<T>::addItem();
		if (callback != nullptr) callback(item);
		return;
	}

	BaseManagerUI::showMenuAndAddItem(isFromAddButton, mouseDownPos, callback);
}

template<class M, class T, class U, class G, class GU>
juce::Component* ManagerUI<M, T, U, G, GU>::getSelectableComponentForBaseItemUI(BaseItemMinimalUI* itemUI)
{
	return getSelectableComponentForItemUI((U*)itemUI);
}

template<class M, class T, class U, class G, class GU>
juce::Component* ManagerUI<M, T, U, G, GU>::getSelectableComponentForItemUI(U* itemUI)
{
	return itemUI;
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::itemRemoved(T* item)
{
	removeItemUI(item);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::itemsRemoved(juce::Array<T*> items)
{
	if (items.size() == 0) return;

	for (auto& i : items) removeItemUI(i, false);

	juce::MessageManager::getInstance()->callAsync([this]()
		{
			resized();
			repaint();
		});

}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::groupRemoved(G* group)
{
	removeItemUI(group);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::groupsRemoved(juce::Array<G*> groups)
{
	if (groups.size() == 0) return;

	for (auto& i : groups) removeItemUI(i, false);

	juce::MessageManager::getInstance()->callAsync([this]()
		{
			resized();
			repaint();
		});
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::newMessage(const typename Manager<T>::ManagerEvent& e)
{
	switch (e.type)
	{
	case Manager<T>::ManagerEvent::ITEM_ADDED:
		if (e.isGroup()) groupAddedAsync(e.getGroup());
		else itemAddedAsync(e.getItem());

		break;

	case Manager<T>::ManagerEvent::ITEMS_REORDERED:
		itemsReorderedAsync();
		break;

	case Manager<T>::ManagerEvent::ITEMS_ADDED:
		itemsAddedAsync(e.getBaseItems());
		break;

	case Manager<T>::ManagerEvent::NEEDS_UI_UPDATE:
		repaint();
		//resized();
		break;

	default:
		break;
	}
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::inspectableDestroyed(Inspectable*)
{
	if (manager != nullptr && !manager->isClearing)
		static_cast<Manager<T>*>(manager)->removeManagerListener(this);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::notifyItemUIAdded(BaseItemMinimalUI* itemUI)
{
	managerUIListeners.call(&ManagerUIListener::itemUIAdded, (U*)itemUI);
}

template<class M, class T, class U, class G, class GU>
void ManagerUI<M, T, U, G, GU>::notifyItemUIRemoved(BaseItemMinimalUI* itemUI)
{
	managerUIListeners.call(&ManagerUIListener::itemUIRemoved, (U*)itemUI);
}
