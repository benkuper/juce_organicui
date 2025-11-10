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
	BaseFactoryDefinition(juce::StringRef menuPath, juce::StringRef type) :
		type(type),
		menuPath(menuPath)
	{
	}

	virtual ~BaseFactoryDefinition() {}

	juce::String type;
	juce::String menuPath;
	juce::Image icon;
	bool isEnabled = true;

	BaseFactoryDefinition* addIcon(juce::Image icon)
	{
		this->icon = icon;
		return this;
	}
	
	virtual const juce::String& getMenuItemName() const { return type; }

	virtual T* create() { jassertfalse; return new T(); }
};

template<class T, typename CreateFunc>
class FactoryDefinition :
	public BaseFactoryDefinition<T>
{
public:
	FactoryDefinition(juce::StringRef menuPath, juce::StringRef type, CreateFunc createFunc) :
		BaseFactoryDefinition<T>(menuPath, type),
		createFunc(createFunc)
	{}

	virtual ~FactoryDefinition() {}

	CreateFunc createFunc;

	static FactoryDefinition* createDef(juce::StringRef menu, juce::StringRef type, CreateFunc createFunc)
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
	FactoryParametricDefinition(juce::StringRef menuPath, juce::StringRef type, CreateFunc createFunc, juce::var params = new juce::DynamicObject()) :
		FactoryDefinition<T, CreateFunc>(menuPath, type, createFunc),
		params(params)
	{
		this->params.getDynamicObject()->setProperty("type", juce::String(type));
	}

	virtual ~FactoryParametricDefinition() {}

	juce::var params;

	FactoryParametricDefinition* addParam(const juce::String& paramName, const juce::var& value)
	{
		params.getDynamicObject()->setProperty(paramName, value);
		return this; //daisy-chain
	}
};

template<class T>
class FactorySimpleParametricDefinition :
	public FactoryParametricDefinition<T, std::function<T* (juce::var)>>
{
public:
	FactorySimpleParametricDefinition(juce::StringRef menuPath, juce::StringRef type, std::function<T* (juce::var)> createFunc, juce::var params = new juce::DynamicObject()) :
		FactoryParametricDefinition<T, std::function<T* (juce::var)>>(menuPath, type, createFunc, params)
	{
	}

	virtual ~FactorySimpleParametricDefinition() {}

	virtual T* createWithExtramParams(juce::var extraParams) {

		if (!extraParams.isObject()) return this->createFunc(this->params);

		juce::var mergedParams = this->params.clone();

		juce::NamedValueSet& props = extraParams.getDynamicObject()->getProperties();
		for (auto& p : props) mergedParams.getDynamicObject()->setProperty(p.name, p.value);

		return this->createFunc(mergedParams);
	}

	virtual T* create() override {
		return this->createFunc(this->params);
	}


	static FactorySimpleParametricDefinition* createDef(juce::StringRef menu, juce::StringRef type, std::function<T* (juce::var)> createFunc, juce::var params = new juce::DynamicObject())
	{
		return new FactorySimpleParametricDefinition(menu, type, createFunc, params);
	}

	template<class S>
	static FactorySimpleParametricDefinition* createDef(juce::StringRef menu)
	{
		return createDef(menu, S::getTypeStringStatic(), &FactorySimpleParametricDefinition<T>::createTemplated<S>, new juce::DynamicObject());
	}

	template<class S>
	static FactorySimpleParametricDefinition* createDef(juce::StringRef menu, juce::StringRef type, juce::var params = new juce::DynamicObject())
	{
		return createDef(menu, type, &FactorySimpleParametricDefinition<T>::createTemplated<S>, params);
	}

	template<class S>
	static T* createTemplated(juce::var params = new juce::DynamicObject()) { return new S(params); }

};

class SubmenuBuilder
{
private:
	struct SubmenuInfo
	{
		juce::String name;
		juce::PopupMenu popupMenu;
		std::vector<SubmenuInfo> children;
		
		juce::PopupMenu* findOrCreateSubmenu(const juce::StringArray& subMenuPath, int depthLevel)
		{
			jassert(!subMenuPath.isEmpty());

			SubmenuInfo* foundMenu = nullptr;
			for (auto& childMenu : children)
			{
				if (childMenu.name == subMenuPath[depthLevel])
				{
					foundMenu = &childMenu;
					break;
				}
			}
			
			if (foundMenu == nullptr)
			{
				SubmenuInfo& newMenu = children.emplace_back();
				newMenu.name = subMenuPath[depthLevel];
				foundMenu = &newMenu;
			}
			
			if (subMenuPath.size() == depthLevel + 1)
			{
				return &foundMenu->popupMenu;
			}

			return foundMenu->findOrCreateSubmenu(subMenuPath, depthLevel + 1);
		}
		
		void addToPopupMenu(juce::PopupMenu* parentMenu)
		{
			for (auto& child : children)
			{
				child.addToPopupMenu(&popupMenu);
			}
			parentMenu->addSubMenu(name, popupMenu);
		}
	};

public:
	juce::PopupMenu* findOrCreateSubmenu(const juce::StringArray& subMenuPath)
	{
		SubmenuInfo* foundSubMenu = nullptr;
		for (auto& childMenu : rootMenus)
		{
			if (childMenu.name == subMenuPath[0])
			{
				foundSubMenu = &childMenu;
				break;
			}
		}

		if (foundSubMenu == nullptr)
		{
			SubmenuInfo& newSubmenu = rootMenus.emplace_back();
			newSubmenu.name = subMenuPath[0];
			foundSubMenu = &newSubmenu;
		}

		if (subMenuPath.size() <= 1)
		{
			return &foundSubMenu->popupMenu;
		}

		return foundSubMenu->findOrCreateSubmenu(subMenuPath, 1);
	}
	
	void build(juce::PopupMenu* parentMenu)
	{
		for (auto& childMenu : rootMenus)
		{
			childMenu.addToPopupMenu(parentMenu);
		}
	}

private:
	std::vector<SubmenuInfo> rootMenus;
};

template<class T>
class Factory
{
public:
	Factory() {}
	virtual ~Factory() {}

	juce::OwnedArray<BaseFactoryDefinition<T>> defs;
	juce::PopupMenu menu;

	virtual void buildPopupMenu(int startOffset = 0)
	{
		menu.clear();

		SubmenuBuilder builder;

		for (auto& d : defs)
		{
			int itemID = defs.indexOf(d) + 1 + startOffset;//start at 1+offset for menu

			if (d->menuPath.isEmpty())
			{
				menu.addItem(itemID, d->getMenuItemName(), d->isEnabled, false, d->icon);
				continue;
			}

			juce::StringArray itemMenuPath = juce::StringArray::fromTokens(d->menuPath, "/", "");
			auto itemMenu = builder.findOrCreateSubmenu(itemMenuPath);
			itemMenu->addItem(itemID, d->getMenuItemName(), true, false, d->icon);
		}

		builder.build(&menu);
	}

	virtual void showCreateMenu(std::function<void(T*)> returnFunc)
	{
		if (defs.size() == 1)
		{
			if (T* r = this->createFromMenuResult(1)) returnFunc(r);
			return;
		}

		getMenu().showMenuAsync(juce::PopupMenu::Options(), [this, returnFunc](int result)
			{
				if (T* r = this->createFromMenuResult(result)) returnFunc(r);
			}
		);
	}

	virtual juce::PopupMenu getMenu(int startOffset = 0)
	{
		if (menu.getNumItems() == 0) buildPopupMenu(startOffset);
		return menu;
	}

	virtual T* createFromMenuResult(int result)
	{
		if (result <= 0 || result > defs.size()) return nullptr;
		else
		{
			BaseFactoryDefinition<T>* d = defs[result - 1];//result 0 is no result
			return create(d);
		}
	}

	virtual T* create(const juce::String& type)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return create(d);
		}

		LOGERROR("Type " << type << " does not exist for this manager.");
		return nullptr;
	}

	virtual T* createWithExtraParams(const juce::String& type, juce::var extraParams)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return createWithExtraParams(d, extraParams);
		}

		LOGERROR("Type " << type << " does not exist for this manager.");
		return nullptr;
	}

	virtual BaseFactoryDefinition<T>* getDefFromExtendedType(const juce::String& extendedType)
	{
		if (!extendedType.matchesWildcard("*/*", true)) return nullptr;

		juce::String menuPath = extendedType.substring(0, extendedType.indexOf("/"));
		juce::String type = extendedType.substring(extendedType.indexOf("/") + 1, extendedType.length());

		for (auto& d : defs)
		{
			if (d->menuPath == menuPath && d->type == type) return d;
		}
		return nullptr;
	}

	virtual T* create(BaseFactoryDefinition<T>* def)
	{
		return def->create();
	}

	virtual T* createWithExtraParams(BaseFactoryDefinition<T>* def, juce::var extraParams)
	{
		return dynamic_cast<FactorySimpleParametricDefinition<T>*>(def)->createWithExtramParams(extraParams);
	}

	bool hasDefinitionWithType(const juce::String& type)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return true;
		}

		return false;
	}

	typedef FactorySimpleParametricDefinition<T> Definition;
};
