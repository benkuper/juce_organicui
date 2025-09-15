/*
  ==============================================================================

	Factory.h
	Created: 7 May 2017 2:43:16pm
	Author:  Ben

  ==============================================================================
*/

#pragma once
class BaseItem;

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

	BaseFactoryDefinition* addIcon(juce::Image _icon)
	{
		this->icon = _icon;
		return this;
	}

	virtual BaseItem* create() { jassertfalse; return nullptr; }
};

template<typename CreateFunc>
class FactoryDefinition :
	public BaseFactoryDefinition
{
public:
	FactoryDefinition(juce::StringRef menuPath, juce::StringRef type, CreateFunc createFunc) :
		BaseFactoryDefinition(menuPath, type),
		createFunc(createFunc)
	{
	}

	virtual ~FactoryDefinition() {}

	CreateFunc createFunc;

	static FactoryDefinition* createDef(juce::StringRef menu, juce::StringRef type, CreateFunc createFunc)
	{
		FactoryDefinition* d = new FactoryDefinition(menu, type, createFunc);
		return d;
	}
};

template<typename CreateFunc>
class FactoryParametricDefinition :
	public FactoryDefinition<CreateFunc>
{
public:


	FactoryParametricDefinition(juce::StringRef menuPath, juce::StringRef type, CreateFunc createFunc, juce::var params = new juce::DynamicObject()) :
		FactoryDefinition<CreateFunc>(menuPath, type, createFunc),
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

class FactorySimpleParametricDefinition :
	public FactoryParametricDefinition<std::function<BaseItem* (juce::var)>>
{
public:
	FactorySimpleParametricDefinition(juce::StringRef menuPath, juce::StringRef type, std::function<BaseItem* (juce::var)> createFunc, juce::var params = new juce::DynamicObject()) :
		FactoryParametricDefinition<std::function<BaseItem* (juce::var)>>(menuPath, type, createFunc, params)
	{
	}

	virtual ~FactorySimpleParametricDefinition() {}

	virtual BaseItem* createWithExtramParams(juce::var extraParams) {

		if (!extraParams.isObject()) return this->createFunc(this->params);

		juce::var mergedParams = this->params.clone();

		juce::NamedValueSet& props = extraParams.getDynamicObject()->getProperties();
		for (auto& p : props) mergedParams.getDynamicObject()->setProperty(p.name, p.value);

		return this->createFunc(mergedParams);
	}

	virtual BaseItem* create() override {
		return this->createFunc(this->params);
	}


	static FactorySimpleParametricDefinition* createDef(juce::StringRef menu, juce::StringRef type, std::function<BaseItem* (juce::var)> createFunc, juce::var params = new juce::DynamicObject())
	{
		return new FactorySimpleParametricDefinition(menu, type, createFunc, params);
	}

	template<class S>
	static FactorySimpleParametricDefinition* createDef(juce::StringRef menu)
	{
		return createDef(menu, S::getTypeStringStatic(), &FactorySimpleParametricDefinition::createTemplated<S>, new juce::DynamicObject());
	}

	template<class S>
	static FactorySimpleParametricDefinition* createDef(juce::StringRef menu, juce::StringRef type, juce::var params = new juce::DynamicObject())
	{
		return createDef(menu, type, &FactorySimpleParametricDefinition::createTemplated<S>, params);
	}

	template<class S>
	static BaseItem* createTemplated(juce::var params = new juce::DynamicObject()) { return new S(params); }

};

class Factory
{
public:

	Factory() {}
	virtual ~Factory() {}

	juce::OwnedArray<BaseFactoryDefinition> defs;
	juce::PopupMenu menu;

	virtual void buildPopupMenu(int startOffset = 0)
	{
		menu.clear();
		juce::OwnedArray<juce::PopupMenu> subMenus;
		juce::Array<juce::String> subMenuNames;

		buildPopupMenuInternal();

		for (auto& d : defs)
		{
			int itemID = defs.indexOf(d) + 1 + startOffset;//start at 1+offset for menu

			if (d->menuPath.isEmpty())
			{
				menu.addItem(itemID, d->type, d->isEnabled, false, d->icon);
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
				subMenus.add(new juce::PopupMenu());
				subMenuIndex = subMenus.size() - 1;
			}

			subMenus[subMenuIndex]->addItem(itemID, d->type, d->isEnabled, false, d->icon);
		}

		for (int i = 0; i < subMenus.size(); ++i) menu.addSubMenu(subMenuNames[i], *subMenus[i]);
	}

	virtual void buildPopupMenuInternal() {}

	virtual void showCreateMenu(std::function<void(BaseItem*)> returnFunc)
	{
		if (defs.size() == 1)
		{
			if (BaseItem* r = this->createFromMenuResult(1)) returnFunc(r);
			return;
		}

		getMenu().showMenuAsync(juce::PopupMenu::Options(), [this, returnFunc](int result)
			{
				if (BaseItem* r = this->createFromMenuResult(result)) returnFunc(r);
			}
		);
	}

	virtual juce::PopupMenu getMenu(int startOffset = 0)
	{
		if (menu.getNumItems() == 0) buildPopupMenu(startOffset);
		return menu;
	}

	virtual BaseItem* createFromMenuResult(int result)
	{
		if (result <= 0 || result > defs.size()) return nullptr;
		else
		{
			BaseFactoryDefinition* d = defs[result - 1];//result 0 is no result
			return create(d);
		}
	}

	virtual BaseItem* create(const juce::String& type)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return create(d);
		}

		LOGERROR("Type " << type << " does not exist for this manager.");
		return nullptr;
	}

	virtual BaseItem* createWithExtraParams(const juce::String& type, juce::var extraParams)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return createWithExtraParams(d, extraParams);
		}

		LOGERROR("Type " << type << " does not exist for this manager.");
		return nullptr;
	}

	virtual BaseFactoryDefinition* getDefFromExtendedType(const juce::String& extendedType)
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

	virtual BaseItem* create(BaseFactoryDefinition* def)
	{
		return def->create();
	}

	virtual BaseItem* createWithExtraParams(BaseFactoryDefinition* def, juce::var extraParams)
	{
		return dynamic_cast<FactorySimpleParametricDefinition*>(def)->createWithExtramParams(extraParams);
	}

	bool hasDefinitionWithType(const juce::String& type)
	{
		for (auto& d : defs)
		{
			if (d->type == type) return true;
		}

		return false;
	}

	typedef FactorySimpleParametricDefinition Definition;
};