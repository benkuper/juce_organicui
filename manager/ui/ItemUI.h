/*
  ==============================================================================

	BaseItemMinimalUI.h
	Created: 20 Nov 2016 2:48:09pm
	Author:  Ben Kuper

  ==============================================================================
*/

#pragma once

template<class T>
class ItemMinimalUI :
	public BaseItemMinimalUI
{
public:

	static_assert(std::is_base_of<BaseItem, T>::value, "T must be derived from BaseItem");

	ItemMinimalUI(T* item) :
		BaseItemMinimalUI(item),
		item(item)
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
	ItemUI(T* item, Direction resizeDirection = NONE, bool showMiniModeBT = false) :
		BaseItemUI(item, resizeDirection, showMiniModeBT),
		item(item)
	{
	}

	virtual ~ItemUI() {}

	T* item;
};

template<class G>
class ItemGroupMinimalUI :
	public BaseItemMinimalUI
{
public:
	static_assert(std::is_base_of<BaseItem, G>::value, "G must be derived from BaseItem");

	ItemGroupMinimalUI(G* item) :
		BaseItemMinimalUI(item),
		group(item)
	{
	}
	virtual ~ItemGroupMinimalUI() {}

	G* group;
};

template<class G>
class ItemGroupUI :
	public BaseItemUI
{
public:
	static_assert(std::is_base_of<BaseItem, G>::value, "G must be derived from BaseItem");

	ItemGroupUI(G* item, Direction resizeDirection = VERTICAL, bool showMiniModeBT = false) :
		BaseItemUI(item, resizeDirection, showMiniModeBT),
		group(item)
	{
		this->setSize(100, 150);
	}

	virtual ~ItemGroupUI() {}
	G* group;

	void resizedInternalHeader(juce::Rectangle<int>& r) override
	{
		if (this->groupManagerUI == nullptr) return;
		if (this->groupManagerUI->addItemBT != nullptr) this->groupManagerUI->addItemBT->setBounds(r.removeFromRight(r.getHeight()));
	}
};