/*
  ==============================================================================

	Factory.h
	Created: 7 May 2017 2:43:16pm
	Author:  Ben

  ==============================================================================
*/

#pragma once

template<class T>
class BaseFactoryDefinition
{
public:
	BaseFactoryDefinition(StringRef menuPath, StringRef type) :
		type(type),
		menuPath(menuPath)
	{
	}

	virtual ~BaseFactoryDefinition() {}

	String type;
	String menuPath;
	Image icon;

	BaseFactoryDefinition* addIcon(Image icon)
	{
		this->icon = icon;
		return this;
	}

	virtual T* create() { jassertfalse; return new T(); }
};

template<class T, typename CreateFunc>
class FactoryDefinition :
	public BaseFactoryDefinition<T>
{
public:
	FactoryDefinition(StringRef menuPath, StringRef type, CreateFunc createFunc) :
		BaseFactoryDefinition<T>(menuPath, type),
		createFunc(createFunc)
	{}

	virtual ~FactoryDefinition() {}

	CreateFunc createFunc;

	static FactoryDefinition* createDef(StringRef menu, StringRef type, CreateFunc createFunc)
	{
		FactoryDefinition* d = new FactoryDefinition(menu, type, createFunc);
		return d;
	}
};

template<class T, typename CreateFunc>
class FactoryParametricDefinition :
	public FactoryDefinition<T, CreateFunc>
{
public:
	FactoryParametricDefinition(StringRef menuPath, StringRef type, CreateFunc createFunc, var params = new DynamicObject()) :
		FactoryDefinition<T, CreateFunc>(menuPath, type, createFunc),
		params(params)
	{
		this->params.getDynamicObject()->setProperty("type", String(type));
	}

	virtual ~FactoryParametricDefinition() {}

	var params;

	FactoryParametricDefinition* addParam(const String& paramName, const var& value)
	{
		params.getDynamicObject()->setProperty(paramName, value);
		return this; //daisy-chain
	}
};

template<class T>
class FactorySimpleParametricDefinition :
	public FactoryParametricDefinition<T, std::function<T *(var)>>
{
public:
	FactorySimpleParametricDefinition(StringRef menuPath, StringRef type, std::function<T *(var)> createFunc, var params = new DynamicObject()) :
		FactoryParametricDefinition<T, std::function<T *(var)>>(menuPath, type, createFunc, params)
	{
	}

	virtual ~FactorySimpleParametricDefinition() {}

	virtual T* create() override {
		return this->createFunc(this->params);
	}

	
	static FactorySimpleParametricDefinition* createDef(StringRef menu, StringRef type, std::function<T* (var)> createFunc, var params = new DynamicObject())
	{
		return new FactorySimpleParametricDefinition(menu, type, createFunc, params);
	}

	template<class S>
	static FactorySimpleParametricDefinition* createDef(StringRef menu)
	{
		return createDef(menu, S::getTypeStringStatic(), &FactorySimpleParametricDefinition<T>::createTemplated<S>, new DynamicObject());
	}

	template<class S>
	static FactorySimpleParametricDefinition* createDef(StringRef menu, StringRef type, var params = new DynamicObject())
	{
		return createDef(menu, type, &FactorySimpleParametricDefinition<T>::createTemplated<S>, params);
	}

	template<class S>
	static T* createTemplated(var params = new DynamicObject()) { return new S(params); }

};


template<class T>
class Factory
{
public:
	Factory() {}
	virtual ~Factory() {}

	OwnedArray<BaseFactoryDefinition<T>> defs;
	PopupMenu menu;

	virtual void buildPopupMenu()
	{
		menu.clear();
		OwnedArray<PopupMenu> subMenus;
		Array<String> subMenuNames;

		for (auto& d : defs)
		{
			int itemID = defs.indexOf(d) + 1;//start at 1 for menu

			if (d->menuPath.isEmpty())
			{
				menu.addItem(itemID, d->type, true, false, d->icon);
				continue;
			}

			int subMenuIndex = -1;

			for (int i = 0; i < subMenus.size(); ++i)
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

			subMenus[subMenuIndex]->addItem(itemID, d->type, true, false, d->icon);
		}

		for (int i = 0; i < subMenus.size(); ++i) menu.addSubMenu(subMenuNames[i], *subMenus[i]);
	}

	virtual void showCreateMenu(std::function<void(T*)> returnFunc)
	{
		if (defs.size() == 1)
		{
			if (T* r = this->createFromMenuResult(1)) returnFunc(r);
			return;
		}

		getMenu().showMenuAsync(PopupMenu::Options(), [this, returnFunc](int result)
			{
				if (T* r = this->createFromMenuResult(result)) returnFunc(r);
			}
		);
	}

	virtual PopupMenu getMenu()
	{
		if (menu.getNumItems() == 0) buildPopupMenu();
		return menu;
	}

	virtual T* createFromMenuResult(int result)
	{
		if (result <= 0 || result > defs.size()) return nullptr;
		else
		{
			BaseFactoryDefinition<T> * d = defs[result - 1];//result 0 is no result
			return create(d);
		}
	}

	virtual T* create(const String& type)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return create(d);
		}

		LOGERROR("Type " << type << " does not exist for this manager.");
		return nullptr;
	}

	virtual T* create(BaseFactoryDefinition<T> * def)
	{
		return def->create();
	}

	bool hasDefinitionWithType(const String& type)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return true;
		}

		return false;
	}

	typedef FactorySimpleParametricDefinition<T> Definition;
};
