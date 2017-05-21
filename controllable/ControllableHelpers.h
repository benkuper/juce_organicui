/*
  ==============================================================================

    ControllableHelpers.h
    Created: 12 May 2016 4:21:18pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLABLEHELPERS_H_INCLUDED
#define CONTROLLABLEHELPERS_H_INCLUDED


class ControllableChooserPopupMenu : 
	public PopupMenu
{
public:
	ControllableChooserPopupMenu(ControllableContainer * rootContainer, bool showParameters = true, bool showTriggers = true);
	virtual ~ControllableChooserPopupMenu();

	bool showParameters;
	bool showTriggers;

	Array<Controllable *> controllableList;
	void populateMenu(PopupMenu *subMenu, ControllableContainer * container, int &currentId);

	Controllable * showAndGetControllable();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableChooserPopupMenu)
};

/*
class ControllableChooser : public Component, public ButtonListener
{
public :
	ControllableChooser(ControllableContainer * rootContainer = nullptr);
	virtual ~ControllableChooser();

	ControllableContainer * rootContainer;

	Controllable * currentControllable;
	void setCurrentControllale(Controllable * c);

	class  Listener
	{
	public:
		virtual ~Listener() {}
		virtual void choosedControllableChanged(Controllable *) {};
	};

	ListenerList<Listener> listeners;
	void addControllableChooserListener(Listener* newListener) { listeners.add(newListener); }
	void removeControllableChooserListener(Listener* listener) { listeners.remove(listener); }
};
*/

//Comparator class to sort controllable array by name
class ControllableComparator
{
public:
	ControllableComparator() {}
	int compareElements(Controllable* c1, Controllable* c2);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllableComparator)
};


class ControllableUtil
{
public:
	static var createDataForParam(const String &type, const String &name, const String &description, var value, var minVal = var(), var maxVal = var(), bool editable = true, bool hiddenInEditor = false);
};

#endif  // CONTROLLABLEHELPERS_H_INCLUDED
