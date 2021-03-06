
juce_ImplementSingleton(Detective);

Detective::Detective() :
	BaseManager("The Detective")
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
		w = addItem(c->getDetectiveWatcher());
		w->target->setValueFromTarget(c);
	}

	ShapeShifterManager::getInstance()->showContent("The Detective");
}

ControllableDetectiveWatcher * Detective::getItemForControllable(Controllable* c)
{
	for (auto& i : items) if (i->controllable == c) return i;
	return nullptr;
}
