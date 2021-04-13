/*
  ==============================================================================

    Outliner.h
    Created: 7 Oct 2016 10:31:23am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class OutlinerItem;
class  OutlinerItemListener
{
public:
	/** Destructor. */
	virtual ~OutlinerItemListener() {}
	virtual void itemNameChanged() {}
	virtual void hideRemoteChanged() {}
};

class OutlinerItemComponent : 
	public InspectableContentComponent, 
	public SettableTooltipClient,
	public ControllableContainerListener,
	public OutlinerItemListener,
	public Label::Listener,
	public Button::Listener
{
public:
	OutlinerItemComponent(OutlinerItem * item);
	~OutlinerItemComponent();

	WeakReference<OutlinerItem> item;
	Label label;
	Colour color;

	std::unique_ptr<ImageButton> hideInRemoteBT;
	
	void paint(Graphics &g) override;
	void resized() override;
	void itemNameChanged() override;
	void hideRemoteChanged() override;

	void buttonClicked(Button* b) override;

	void labelTextChanged(Label*) override;
	void mouseDown(const MouseEvent &e) override;

	void inspectableSelectionChanged(Inspectable * i) override;


};

class OutlinerItem :
	public TreeViewItem,
	public InspectableContent,
	public ControllableContainerListener
{
public:
	OutlinerItem(WeakReference<ControllableContainer> container, bool parentsHaveHideInRemote, bool isFiltered);
	OutlinerItem(WeakReference<Controllable> controllable, bool parentsHaveHideInRemote, bool isFiltered);
	~OutlinerItem();

	bool isContainer;
	String itemName;
	bool parentsHaveHideInRemote;
	bool isFiltered;

	WeakReference<ControllableContainer> container;
	WeakReference<Controllable> controllable;

	virtual bool mightContainSubItems() override;

	Component * createItemComponent() override;

	String getUniqueName() const override;
	void inspectableSelectionChanged(Inspectable * inspectable) override;

	void setHideInRemote(bool value);
	void setParentsHaveHideInRemote(bool value);
	
	void childAddressChanged(ControllableContainer *) override;

	ListenerList<OutlinerItemListener> itemListeners;
	void addItemListener(OutlinerItemListener* newListener) { itemListeners.add(newListener); }
	void removeItemListener(OutlinerItemListener* listener) { itemListeners.remove(listener); }

	WeakReference<OutlinerItem>::Master masterReference;
};

class Outliner : public ShapeShifterContentComponent,
	public ControllableContainerListener,
	public Label::Listener,
	public Button::Listener,
	public EngineListener
{
public:
	juce_DeclareSingleton(Outliner, true)

	Outliner(const String &contentName = "");
	~Outliner();

	Label searchBar;
	bool listIsFiltered;
	
	std::unique_ptr<ImageButton> remoteHideShowAllBT;
	bool hideShowState;

	TreeView treeView;
	std::unique_ptr<OutlinerItem> rootItem;
	bool enabled; //update or not

	Array<ControllableContainer*> parentsOfFiltered;
	Array<ControllableContainer*> filteredContainers;
	Array<Inspectable *> childOfFiltered;
	Array<Controllable*> filteredControllables;

	void clear();
	
	void setEnabled(bool value);

	void resized() override;
	void paint(Graphics &g) override;


	void rebuildTree(ControllableContainer * fromContainer = nullptr);
	void buildTree(OutlinerItem * parentItem, ControllableContainer * parentContainer, bool parentsHaveHideInRemote);
	void updateFilteredList();

	void endLoadFile() override;
	void engineCleared() override;

	OutlinerItem* getItemForContainer(ControllableContainer* cc);

	void childStructureChanged(ControllableContainer *) override;

	static Outliner * create(const String &contentName) { return new Outliner(contentName); }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Outliner)

	virtual void labelTextChanged(Label* labelThatHasChanged) override;
	virtual void buttonClicked(Button* b) override;
};