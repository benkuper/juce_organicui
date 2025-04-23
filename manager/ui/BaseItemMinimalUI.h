/*
  ==============================================================================

	BaseItemMinimalUI.h
	Created: 20 Nov 2016 2:48:09pm
	Author:  Ben Kuper

  ==============================================================================
*/

#pragma once

class BaseItemMinimalUI :
	public InspectableContentComponent,
	public ContainerAsyncListener,
	public juce::DragAndDropContainer,
	public juce::DragAndDropTarget
{
public:

	BaseItemMinimalUI(BaseItem* _item);
	virtual ~BaseItemMinimalUI();

	BaseItem* baseItem;

	//ui
	juce::Colour bgColor;
	juce::Colour selectedColor;

	bool syncWithItemSize;

	float viewZoom;
	float viewCheckerSize;

	bool dimAlphaOnDisabled;
	bool highlightOnMouseOver;
	bool fillColorOnSelected;

	//Dragging
	bool dragAndDropEnabled; //tmp, waiting to implement full drag&drop system
	int dragStartDistance = 10;
	bool autoHideWhenDragging;
	bool drawEmptyDragIcon;

	//dropping
	juce::StringArray acceptedDropTypes;
	bool isDraggingOver;
	bool highlightOnDragOver;


	void setHighlightOnMouseOver(bool highlight);
	void paint(juce::Graphics& g) override;
	void setViewZoom(float value);
	void setViewCheckerSize(float value);

	void setViewSize(float x, float y);
	void setViewSize(juce::Point<float> size);

	virtual void updateItemUISize();

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseDrag(const juce::MouseEvent& e) override;
	virtual void mouseExit(const juce::MouseEvent& e) override;

	virtual bool isUsingMouseWheel();

	virtual void selectToThis() override;

	virtual void addContextMenuItems(juce::PopupMenu& p) {}
	virtual void handleContextMenuResult(int result) {}

	virtual void newMessage(const ContainerAsyncEvent& e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer*) {}
	virtual void controllableFeedbackUpdateInternal(Controllable*);
	virtual void controllableStateUpdateInternal(Controllable*) {}

	//Drag drop container
	virtual bool canStartDrag(const juce::MouseEvent& e);
	virtual void dragOperationStarted(const juce::DragAndDropTarget::SourceDetails&) override;
	virtual void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails&) override;

	//Drag drop target
	virtual bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

	virtual void itemDragEnter(const SourceDetails&) override;
	virtual void itemDragExit(const SourceDetails&) override;
	virtual void itemDropped(const SourceDetails& dragSourceDetails) override; //to be overriden
	virtual juce::Point<int> getDragOffset();

	bool isGroupUI();

	class ItemMinimalUIListener
	{
	public:
		virtual ~ItemMinimalUIListener() {}

		virtual void itemUIViewPositionChanged(BaseItemMinimalUI*) {}

		virtual void itemUIResizeDrag(BaseItemMinimalUI*, const juce::Point<int>& dragOffset) {}
		virtual void itemUIResizeEnd(BaseItemMinimalUI*) {}

		virtual void askForSyncPosAndSize(BaseItemMinimalUI*) {}
		virtual void askSelectToThis(BaseItemMinimalUI*) {}
	};

	juce::ListenerList<ItemMinimalUIListener> itemMinimalUIListeners;
	void addItemMinimalUIListener(ItemMinimalUIListener* newListener) { itemMinimalUIListeners.add(newListener); }
	void removeItemMinimalUIListener(ItemMinimalUIListener* listener) { itemMinimalUIListeners.remove(listener); }
};
