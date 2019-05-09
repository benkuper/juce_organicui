/*
  ==============================================================================

    Factory.h
    Created: 7 May 2017 2:43:16pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

template<class T>
class Factory
{
public:
	class Definition
	{
	public:
		String type;
		String menuPath;
		std::function<T*(var)> createFunc;

		var params;

		Definition(const String &menuPath, const String &type, std::function<T*(var)> createFunc) :
			type(type),
			menuPath(menuPath),
			createFunc(createFunc)
		{  
			params = var(new DynamicObject());
			params.getDynamicObject()->setProperty("type", type);
		}

		static Definition * createDef(const String &menu, const String &type, std::function<T*(var)> createFunc)
		{
			Definition * d = new Definition(menu, type, createFunc);
			return d;
		}

		Definition * addParam(const String &paramName, const var &value)
		{
			params.getDynamicObject()->setProperty(paramName, value);
			return this; //daisy-chain
		}

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Definition)
	};



	OwnedArray<Definition> defs;
	PopupMenu menu;

	virtual void buildPopupMenu()
	{
		menu.clear();
		OwnedArray<PopupMenu> subMenus;
		Array<String> subMenuNames;

		for (auto &d : defs)
		{
			int itemID = defs.indexOf(d) + 1;//start at 1 for menu

			if (d->menuPath.isEmpty())
			{
				menu.addItem(itemID, d->type);
				continue;
			}

			int subMenuIndex = -1;

			for (int i = 0; i < subMenus.size(); i++)
			{
				if (subMenuNames[i] == d->menuPath)
				{
					subMenuIndex = i;
					break;
				}
			}
			if (subMenuIndex == -1)
			{
				subMenuNames.add(d->menuPath);
				subMenus.add(new PopupMenu());
				subMenuIndex = subMenus.size() - 1;
			}

			subMenus[subMenuIndex]->addItem(itemID, d->type);
		}

		for (int i = 0; i < subMenus.size(); i++) menu.addSubMenu(subMenuNames[i], *subMenus[i]);
	}

	virtual T * showCreateMenu()
	{
		int result = getMenu().getNumItems() == 1 ? 1 : getMenu().show();
		return createFromMenuResult(result);
	}

	virtual PopupMenu getMenu()
	{
		if (menu.getNumItems() == 0) buildPopupMenu();
		return menu;
	}

	virtual T * createFromMenuResult(int result)
	{
		if (result <= 0 || result > defs.size()) return nullptr;
		else
		{
			Definition * d = defs[result - 1];//result 0 is no result
			return create(d->type);
		}
	}


	virtual T * create(const String &type)
	{
		for (auto &d : defs)
		{
			if (d->type == type) return d->createFunc(d->params);
		}
		return nullptr;
	}
};
