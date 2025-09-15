
juce_ImplementSingleton(Detective);

Detective::Detective() :
	Manager("The Detective")
{
	userCanAddItemsManually = false;
	isSelectable = false;
}

Detective::~Detective()
{
}

void Detective::watchControllable(Controllable* c)
{
	ControllableDetectiveWatcher* w = getItemForControllable(c);
	if (w == nullptr)
	{
		w = c->getDetectiveWatcher();
		addItem(w);
		w->target->setValueFromTarget(c);
	}

	ShapeShifterManager::getInstance()->showContent("The Detective");
}

ControllableDetectiveWatcher * Detective::getItemForControllable(Controllable* c)
{
	Array<ControllableDetectiveWatcher*> items = getItems();
	for (auto& i : items) if (i->controllable == c) return i;
	return nullptr;
}
