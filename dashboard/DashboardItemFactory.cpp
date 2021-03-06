juce_ImplementSingleton(DashboardItemFactory)

DashboardItemFactory::DashboardItemFactory()
{
	defs.add(Definition::createDef("", "DashboardTriggerItem",&DashboardTriggerItem::create));
	defs.add(Definition::createDef("", "DashboardParameterItem", &DashboardParameterItem::create));
	defs.add(Definition::createDef("", "DashboardCCItem", &DashboardCCItem::create));
	defs.add(Definition::createDef("", "DashboardGroupItem", &DashboardGroupItem::create));
	defs.add(Definition::createDef("", "DashboardCommentItem", &DashboardCommentItem::create));
}

void DashboardItemFactory::buildPopupMenu()
{
	menu.clear();

	int indexOffset = 0;
	for (auto &p : providers)
	{
		menu.addSubMenu(p->getProviderName(), p->getDashboardCreateMenu(indexOffset));
		indexOffset += 10000;
	}
	menu.addSeparator(); 
	for (auto &p : specialProviders)
	{
		menu.addSubMenu(p->getProviderName(), p->getDashboardCreateMenu(indexOffset));
		indexOffset += 10000;
	}
}

DashboardItem * DashboardItemFactory::showCreateMenu()
{
	buildPopupMenu(); //force rebuild
	int result = getMenu().show();
	if (result != 0) return createFromMenuResult(result);
	return nullptr;
}

DashboardItem * DashboardItemFactory::createFromMenuResult(int result)
{
	if (result < 0) return nullptr;

	int providerId = (int)floorf(result / 10000);
	if (providerId < providers.size()) return providers[providerId]->getDashboardItemFromMenuResult(result - providerId * 10000);
	else if (providerId - providers.size() < specialProviders.size()) return specialProviders[providerId - providers.size()]->getDashboardItemFromMenuResult(result - providerId * 10000);
	
	return nullptr;
}
