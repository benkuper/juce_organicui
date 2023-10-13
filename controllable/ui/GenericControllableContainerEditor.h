/*
  ==============================================================================

	GenericControllableContainerEditor.h
	Created: 9 May 2016 6:41:59pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once


class GenericControllableContainerEditor :
	public InspectableEditor,
	public ContainerAsyncListener,
	public juce::Button::Listener,
	public juce::ChangeListener,
	public juce::Label::Listener,
	public juce::DragAndDropContainer
{
public:
	GenericControllableContainerEditor(juce::Array<ControllableContainer*> containers, bool isRoot, bool buildAtCreation = true);
	virtual ~GenericControllableContainerEditor();


	int headerHeight;
	const int headerGap = 4;

	bool isRebuilding; //to avoid constant resizing when rebuilding/adding items

	bool prepareToAnimate;
	juce::ComponentAnimator collapseAnimator;

	juce::Colour contourColor;
	juce::Label containerLabel;

	juce::Array<ControllableContainer*> containers;
	juce::WeakReference<ControllableContainer> container;
	juce::OwnedArray<InspectableEditor> childEditors;

	std::unique_ptr<juce::ImageButton> expandBT;
	std::unique_ptr<juce::ImageButton> collapseBT;
	std::unique_ptr<juce::ImageButton> addBT;
	std::unique_ptr<WarningTargetUI> warningUI;
	std::unique_ptr<juce::ImageButton> removeBT;

	std::function<InspectableEditor* (ControllableContainer* parent, Controllable* c)> customCreateEditorForControllableFunc;
	std::function<InspectableEditor* (ControllableContainer* parent, ControllableContainer* cc)> customCreateEditorForContainerFunc;

	juce::Rectangle<int> dragRect;

	bool dragAndDropEnabled;

	virtual void setCollapsed(bool value, bool force = false, bool animate = true, bool doNotRebuild = false);
	virtual void toggleCollapsedChildren();
	virtual void resetAndBuild();


	void paint(juce::Graphics& g) override;
	void resized() override;
	virtual void resizedInternal(juce::Rectangle<int>& r);
	virtual void resizedInternalHeader(juce::Rectangle<int>& r);
	virtual void resizedInternalContent(juce::Rectangle<int>& r);

	virtual void clear();

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseUp(const juce::MouseEvent& e) override;
	void mouseDrag(const juce::MouseEvent& e) override;
	virtual void setDragDetails(juce::var&) {}

	virtual void setDragAndDropEnabled(bool value);


	virtual void showContextMenu();
	virtual void addPopupMenuItems(juce::PopupMenu*) {} //for child classes
	virtual void handleMenuSelectedID(int) {} // to override


	juce::Rectangle<int> getHeaderBounds();
	juce::Rectangle<int> getContentBounds();

	bool canBeCollapsed();

	virtual InspectableEditor* getEditorUIForControllable(Controllable* c);
	virtual InspectableEditor* addControllableUI(Controllable* c, bool resize = false);
	virtual void removeControllableUI(Controllable* c, bool resize = false);

	virtual InspectableEditor* getEditorUIForContainer(ControllableContainer* cc);
	virtual InspectableEditor* addEditorUI(ControllableContainer* cc, bool resize = false);
	virtual void removeEditorUI(InspectableEditor* i, bool resize = false);

	virtual void showMenuAndAddControllable();

	InspectableEditor* getEditorForInspectable(Inspectable* i);

	virtual void buttonClicked(juce::Button* b) override;
	virtual void labelTextChanged(juce::Label* l) override;

	virtual void componentVisibilityChanged(juce::Component& c) override;

	void newMessage(const ContainerAsyncEvent& p) override;
	virtual void controllableFeedbackUpdate(Controllable*) {};
	void childBoundsChanged(juce::Component*) override;



	// Inherited via ChangeListener
	virtual void changeListenerCallback(juce::ChangeBroadcaster* source) override;

	class  ContainerEditorListener
	{
	public:
		/** Destructor. */
		virtual ~ContainerEditorListener() {}
		virtual void containerRebuilt(GenericControllableContainerEditor*) {}
	};

	juce::ListenerList<ContainerEditorListener> containerEditorListeners;
	void addContainerEditorListener(ContainerEditorListener* newListener) { containerEditorListeners.add(newListener); }
	void removeContainerEditorListener(ContainerEditorListener* listener) { containerEditorListeners.remove(listener); }
};


class EnablingControllableContainerEditor :
	public GenericControllableContainerEditor
{
public:
	EnablingControllableContainerEditor(juce::Array<EnablingControllableContainer*> cc, bool isRoot, bool buildAtCreation = true);
	~EnablingControllableContainerEditor() {}

	juce::Array<EnablingControllableContainer*> ioContainers;
	EnablingControllableContainer* ioContainer;
	std::unique_ptr<BoolToggleUI> enabledUI;

	virtual void resizedInternalHeader(juce::Rectangle<int>& r) override;
	virtual void controllableFeedbackUpdate(Controllable*) override;
};