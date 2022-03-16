/*
  ==============================================================================

	Inspector.cpp
	Created: 9 May 2016 6:41:38pm
	Author:  bkupe

  ==============================================================================
*/

std::function<Inspector* (InspectableSelectionManager*)> InspectorUI::customCreateInspectorFunc = nullptr;

Inspector::Inspector(InspectableSelectionManager* _selectionManager) :
	selectionManager(nullptr),
	currentEditor(nullptr),
	showTextOnEmpty(true)
{

	setSelectionManager(_selectionManager);

	vp.setScrollBarsShown(true, false);
	vp.setScrollOnDragMode(Viewport::ScrollOnDragMode::never);
	vp.setScrollBarThickness(10);
	addAndMakeVisible(vp);

	resized();
}



Inspector::~Inspector()
{
	if (selectionManager != nullptr) selectionManager->removeAsyncSelectionManagerListener(this);
	else if (InspectableSelectionManager::mainSelectionManager != nullptr) InspectableSelectionManager::mainSelectionManager->removeAsyncSelectionManagerListener(this);

	clear();
}


void Inspector::paint(Graphics& g)
{
	if (showTextOnEmpty && currentInspectables.size() == 0 && selectionManager != nullptr)
	{
		g.setColour(Colours::white.withAlpha(.4f));
		g.setFont(jmin(getHeight() - 2, 14));
		String text = "Select an object to edit its parameters here";
		if (text.isNotEmpty()) g.drawFittedText(text, getLocalBounds(), Justification::centred, 3);
	}
}

void Inspector::resized()
{
	juce::Rectangle<int> r = getLocalBounds().reduced(3);
	resizedInternal(r);
}

void Inspector::resizedInternal(juce::Rectangle<int>& r)
{
	vp.setBounds(r);
	r.removeFromRight(10);

	if (currentEditor != nullptr)
	{
		if (!currentEditor->fitToContent) r.setHeight(currentEditor->getHeight());
		currentEditor->setSize(r.getWidth(), r.getHeight());
	}
}

void Inspector::setSelectionManager(InspectableSelectionManager* newSM)
{
	if (selectionManager != nullptr) selectionManager->removeAsyncSelectionManagerListener(this);

	selectionManager = newSM;

	if (selectionManager == nullptr) selectionManager = InspectableSelectionManager::mainSelectionManager;
	if (selectionManager != nullptr) selectionManager->addAsyncSelectionManagerListener(this);
}

void Inspector::setCurrentInspectables(Array<Inspectable*> inspectables, bool setInspectableSelection)
{
	if (!isEnabled()) return;

	bool isSame = inspectables.size() == currentInspectables.size();
	if (inspectables.size() == currentInspectables.size())
	{
		bool isSame = true;
		for (int i = 0; i < inspectables.size(); i++)
		{
			if (inspectables[i] != currentInspectables[i])
			{
				isSame = false;
				break;
			}
		}
		if (isSame) return;
	}

	MessageManagerLock mmLock;

	for (auto& i : currentInspectables)
	{
		if (i != nullptr)
		{
			i->removeInspectableListener(this);
			if (setInspectableSelection) i->setSelected(false);
		}
	}
	if (currentEditor != nullptr)
	{
		vp.setViewedComponent(nullptr);
		currentEditor = nullptr;
	}

	currentInspectables = inspectables;

	for (auto& i : currentInspectables)
	{
		if (setInspectableSelection) i->setSelected(true);
		i->addInspectableListener(this);
	}

	if (currentInspectables.size() > 0)
	{
		currentEditor.reset(currentInspectables[0]->getEditor(true, currentInspectables));

		vp.setViewedComponent(currentEditor.get(), false);
	}
	resized();

	listeners.call(&InspectorListener::currentInspectableChanged, this);
}


void Inspector::clear()
{
	setCurrentInspectables();
}

void Inspector::inspectableDestroyed(Inspectable* i)
{
	if (currentInspectables.contains(i))
	{
		Array<Inspectable*> newInspectables(currentInspectables.getRawDataPointer(), currentInspectables.size());
		newInspectables.removeAllInstancesOf(i);
		setCurrentInspectables(newInspectables);
	}
}

void Inspector::newMessage(const InspectableSelectionManager::SelectionEvent& e)
{
	if (e.type == InspectableSelectionManager::SelectionEvent::SELECTION_CHANGED)
	{
		if (selectionManager->isEmpty())
		{
			if (curSelectionDoesNotAffectInspector) return;
			setCurrentInspectables();
		}
		else
		{
			Inspectable* newI = selectionManager->currentInspectables[0];
			curSelectionDoesNotAffectInspector = !newI->showInspectorOnSelect;
			if (curSelectionDoesNotAffectInspector) return;

			Array<Inspectable*> newInspectables = selectionManager->getInspectablesAs<Inspectable>();
			if (newInspectables.size() > 0 && newInspectables[0] != nullptr && newInspectables[0]->showInspectorOnSelect) setCurrentInspectables(newInspectables);
		}

		repaint();
	}
}

InspectorUI::InspectorUI(const String& name, InspectableSelectionManager* selectionManager) :
	ShapeShifterContentComponent(name)
{
	inspector.reset(customCreateInspectorFunc != nullptr ? customCreateInspectorFunc(selectionManager) : new Inspector(selectionManager));
	addAndMakeVisible(inspector.get());

	helpID = "Inspector";
}

InspectorUI::~InspectorUI()
{
}

void InspectorUI::resized()
{
	inspector->setBounds(getLocalBounds());
}
