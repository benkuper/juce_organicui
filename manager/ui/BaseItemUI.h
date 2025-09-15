/*
  ==============================================================================

	BaseItemUI.h
	Created: 28 Oct 2016 8:04:09pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once


class BaseItemUI :
	public BaseItemMinimalUI,
	public juce::Button::Listener,
	public juce::Label::Listener,
	public juce::ComponentListener,
	public Parameter::AsyncListener
{
public:
	enum Direction { NONE, VERTICAL, HORIZONTAL, ALL };

	BaseItemUI(BaseItem* _item, Direction resizeDirection = NONE, bool showMiniModeBT = false);
	virtual ~BaseItemUI();

	//LAYOUT
	int margin;
	int minContentHeight;

	juce::Point<float> sizeAtMouseDown;

	//header
	int headerHeight;
	int headerGap;
	bool showEnableBT;
	bool showRemoveBT;
	bool showColorUI;

	//Resize
	Direction resizeDirection;
	int resizerWidth;
	int resizerHeight;


	class ItemResizerComponent :
		public juce::Component
	{
	public:
		ItemResizerComponent()
		{
			setMouseCursor(juce::MouseCursor::BottomRightCornerResizeCursor);
		}

		~ItemResizerComponent() {}

		void paint(juce::Graphics& g)
		{
			g.setColour(isMouseOverOrDragging() ? HIGHLIGHT_COLOR : juce::Colours::lightgrey.withAlpha(.3f));
			for (int i = 0; i < 3; i++) g.drawLine(getWidth() * i / 3.f, (float)getHeight(), (float)getWidth(), getHeight() * i / 3.f);
		}
	};

	std::unique_ptr<ItemResizerComponent> cornerResizer;
	std::unique_ptr<juce::ResizableEdgeComponent> edgeResizer;
	juce::ComponentBoundsConstrainer constrainer;

	juce::Component* resizer;

	juce::Label itemLabel;
	std::unique_ptr<BoolToggleUI> enabledBT;
	std::unique_ptr<juce::ImageButton> removeBT;
	std::unique_ptr<ColorParameterUI> itemColorUI;
	std::unique_ptr<WarningTargetUI> warningUI;
	std::unique_ptr<BoolToggleUI> miniModeBT;

	//std::unique_ptr<Grabber> grabber;

	juce::Array<juce::Component*> contentComponents;

	//std::unique_ptr<BaseManager> groupManagerUI; //if it's a group
	bool showGroupManager;

	void setContentSize(int contentWidth, int contentHeight);
	void setShowGroupManager(bool value);

	//minimode
	int getHeightWithoutContent();

	virtual void updateMiniModeUI();
	virtual void updateItemUISize() override;

	virtual void resized() override;
	virtual void resizedHeader(juce::Rectangle<int>& r);
	virtual void resizedInternalHeader(juce::Rectangle<int>&) {}
	virtual void resizedContent(juce::Rectangle<int>&);
	virtual void resizedInternalContent(juce::Rectangle<int>&) {}
	virtual void resizedInternalFooter(juce::Rectangle<int>&) {}

	virtual void updateGroupManagerBounds();

	virtual void buttonClicked(juce::Button* b) override;

	virtual void mouseDown(const juce::MouseEvent& e) override;
	virtual void mouseDrag(const juce::MouseEvent& e) override;
	virtual void mouseUp(const juce::MouseEvent& e) override;

	virtual void labelTextChanged(juce::Label* l) override;
	virtual bool keyPressed(const juce::KeyPress& e) override;

	virtual bool canStartDrag(const juce::MouseEvent& e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer*) override;
	virtual void controllableFeedbackUpdateInternal(Controllable*) override;

	virtual void newMessage(const Parameter::ParameterEvent& e) override;

	void childBoundsChanged(Component* c) override;
	virtual void visibilityChanged() override;

	virtual void componentVisibilityChanged(juce::Component& c) override;

	//virtual BaseManagerUI* createGroupManagerUI();
	//virtual BaseManagerUI* createGroupManagerUIInternal();

	class ItemUIListener
	{
	public:
		virtual ~ItemUIListener() {}
		virtual void itemUIMiniModeChanged(BaseItemUI*) {}
	};

	juce::ListenerList<ItemUIListener> itemUIListeners;
	void addItemUIListener(ItemUIListener* newListener) { itemUIListeners.add(newListener); }
	void removeItemUIListener(ItemUIListener* listener) { itemUIListeners.remove(listener); }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseItemUI)
};

