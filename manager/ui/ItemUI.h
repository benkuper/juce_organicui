/*
  ==============================================================================

	BaseItemMinimalUI.h
	Created: 20 Nov 2016 2:48:09pm
	Author:  Ben Kuper

  ==============================================================================
*/

#pragma once
template<class T> class ItemBaseGroup;

template<class T>
class ItemMinimalUI :
	public BaseItemMinimalUI
{
public:

	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	ItemMinimalUI(T* _item) :
		BaseItemMinimalUI(_item),
		item(_item)
	{
	}

	virtual ~ItemMinimalUI() {}

	T* item;
};

template<class T>
class ItemUI :
	public BaseItemUI
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	ItemUI(T* _item, Direction resizeDirection = NONE, bool showMiniModeBT = false) :
		BaseItemUI(_item, resizeDirection, showMiniModeBT),
		item(_item)
	{
	}

	virtual ~ItemUI() {}

	T* item;
};

template<class T, class G = ItemBaseGroup<T>>
class ItemGroupMinimalUI :
	public BaseItemMinimalUI
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	static_assert(std::is_base_of<ItemBaseGroup<T>, G>::value, "G must be derived from ItemBaseGroup<T>");

	ItemGroupMinimalUI(G* _item) :
		BaseItemMinimalUI(_item),
		group(_item)
	{
	}
	virtual ~ItemGroupMinimalUI() {}
	
	G* group;
};

template<class T, class G = ItemBaseGroup<T>>
class ItemGroupUI :
	public BaseItemUI
{
public:
	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");
	static_assert(std::is_base_of<ItemBaseGroup<T>, G>::value, "G must be derived from ItemBaseGroup<T>");

	ItemGroupUI(G* _item, Direction resizeDirection = NONE, bool showMiniModeBT = false) :
		BaseItemUI(_item, resizeDirection, showMiniModeBT),
		group(_item)
	{
	}

	virtual ~ItemGroupUI() {}
	G* group;
};