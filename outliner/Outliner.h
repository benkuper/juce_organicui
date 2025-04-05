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
	public juce::SettableTooltipClient,
	public ControllableContainerListener,
	public OutlinerItemListener,
	public juce::Label::Listener,
	public juce::Button::Listener
{
public:
	OutlinerItemComponent(OutlinerItem * item);
	virtual ~OutlinerItemComponent();

	juce::WeakReference<OutlinerItem> item;
	juce::Label label;
	juce::Colour color;

	std::unique_ptr<juce::ImageButton> hideInRemoteBT;
	
	virtual void paint(juce::Graphics &g) override;
	virtual void resized() override;
	virtual void resizedInternal(juce::Rectangle<int>& r) {}

	void itemNameChanged() override;
	void hideRemoteChanged() override;

	virtual void buttonClicked(juce::Button* b) override;

	void labelTextChanged(juce::Label*) override;
	virtual void mouseDown(const juce::MouseEvent &e) override;

	void inspectableSelectionChanged(Inspectable * i) override;
};

class OutlinerItem :
	public juce::TreeViewItem,
	public InspectableContent,
	public ControllableContainerListener
{
public:
	OutlinerItem(juce::WeakReference<ControllableContainer> container, bool parentsHaveHideInRemote, bool isFiltered);
	OutlinerItem(juce::WeakReference<Controllable> controllable, bool parentsHaveHideInRemote, bool isFiltered);
	virtual ~OutlinerItem();

	bool isContainer;
	juce::String itemName;
	bool parentsHaveHideInRemote;
	bool isFiltered;

	juce::WeakReference<ControllableContainer> container;
	juce::WeakReference<Controllable> controllable;

	virtual bool mightContainSubItems() override;

	virtual std::unique_ptr<juce::Component> createItemComponent() override;

	juce::String getUniqueName() const override;
	void inspectableSelectionChanged(Inspectable * inspectable) override;

	void setHideInRemote(bool value);
	void setParentsHaveHideInRemote(bool value);
	
	void childAddressChanged(ControllableContainer *) override;


	juce::ListenerList<OutlinerItemListener> itemListeners;
	void addItemListener(OutlinerItemListener* newListener) { itemListeners.add(newListener); }
	void removeItemListener(OutlinerItemListener* listener) { itemListeners.remove(listener); }

	juce::WeakReference<OutlinerItem>::Master masterReference;
};

class Outliner : public ShapeShifterContentComponent,
	public ControllableContainerListener,
	public juce::Label::Listener,
	public juce::Button::Listener,
	public EngineListener,
	public Timer
{
public:
	juce_DeclareSingleton(Outliner, true)

	Outliner(const juce::String &contentName = "");
	virtual ~Outliner();

	juce::Label searchBar;
	bool listIsFiltered;
	
	std::unique_ptr<juce::ImageButton> remoteHideShowAllBT;
	bool hideShowState;

	juce::TreeView treeView;
	std::unique_ptr<OutlinerItem> rootItem;
	bool enabled; //update or not
	bool shouldRebuild;

	juce::Array<ControllableContainer*> parentsOfFiltered;
	juce::Array<ControllableContainer*> filteredContainers;
	juce::Array<Inspectable *> childOfFiltered;
	juce::Array<Controllable*> filteredControllables;

	void clear();
	
	void setEnabled(bool value);

	void resized() override;
	void paint(juce::Graphics &g) override;


	void rebuildTree(ControllableContainer * fromContainer = nullptr);
	void buildTree(OutlinerItem * parentItem, ControllableContainer * parentContainer, bool parentsHaveHideInRemote);
	void updateFilteredList();

	virtual OutlinerItem* createItem(juce::WeakReference<ControllableContainer> container, bool parentsHaveHideInRemote, bool isFiltered);
	virtual OutlinerItem* createItem(juce::WeakReference<Controllable> controllable, bool parentsHaveHideInRemote, bool isFiltered);

	void fileLoaded() override;
	void engineCleared() override;

	OutlinerItem* getItemForContainer(ControllableContainer* cc);

	void childStructureChanged(ControllableContainer *) override;

	void timerCallback() override;


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Outliner)

	virtual void labelTextChanged(juce::Label* labelThatHasChanged) override;
	virtual void buttonClicked(juce::Button* b) override;
};