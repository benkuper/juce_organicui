/*
  ==============================================================================

    BaseItem.h
    Created: 28 Oct 2016 8:04:25pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class GenericControllableManager;
class ScriptManager;
class Script;

class BaseItem :
	public EnablingControllableContainer,
	public BaseManagerListener<Script>
{
public:
	BaseItem(const String& name = "", bool canBeDisabled = true, bool canHaveScript = false);
	virtual ~BaseItem();

	//UI - should move outside data class ? how to save/load if not there 
	BoolParameter* miniMode;
	FloatParameter* listUISize; //width or height in a list
	Point2DParameter* viewUIPosition; //position in a view
	Point2DParameter* viewUISize; //size in a view
	BoolParameter* isUILocked; //lock in UI
	bool useCustomArrowKeysBehaviour; // to have custom manipulation with arrow keys instead of selection prev/next

	bool canHaveScripts;
	bool userCanRemove;
	bool userCanDuplicate;
	bool askConfirmationBeforeRemove;
	bool isSavable;
	bool saveType;
	bool canBeReorderedInEditor;

	std::unique_ptr<ScriptManager> scriptManager;

	String itemDataType;
	bool isClearing;

	//UI moving X/Y
	Point<float> movePositionReference;


	virtual void clearItem();

	virtual void duplicate();
	virtual void copy();
	virtual bool paste();
	virtual void selectAll(bool addToSelection = false);
	virtual void selectPrevious(bool addToSelection = false);
	virtual void selectNext(bool addToSelection = false);

	virtual void moveBefore(); //list
	virtual void moveAfter(); //list

	void remove();

	void setMovePositionReference(bool setOtherSelectedItems = false);
	virtual void setMovePositionReferenceInternal();
	void movePosition(Point<float> positionOffset, bool moveOtherSelectedItems = false);
	void scalePosition(Point<float> positionOffset, bool moveOtherSelectedItems = false);
	virtual void setPosition(Point<float> position);
	virtual Point<float> getPosition();
	void addMoveToUndoManager(bool addOtherSelectedItems = false);
	virtual void addUndoableMoveAction(Array<UndoableAction *> &arrayToAdd);

	//listeners
	virtual void onContainerParameterChanged(Parameter *) override;
	virtual void onContainerTriggerTriggered(Trigger *) override;
	virtual void onContainerParameterChangedInternal(Parameter *) {} //child classes override this function
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) {};

	void itemAdded(Script* script) override;


	var getJSONData() override;
	void loadJSONDataInternal(var data) override;
	virtual void loadJSONDataItemInternal(var data) {} //happens before loading scripts

	InspectableEditor * getEditor(bool isRoot) override;
	virtual String getTypeString() const { return "BaseItem"; };
	static var getTypeStringFromScript(const juce::var::NativeFunctionArgs& a);
	String getScriptTargetString() override;

	ListenerList<BaseItemListener> baseItemListeners;
	void addBaseItemListener(BaseItemListener* newListener) { baseItemListeners.add(newListener); }
	void removeBaseItemListener(BaseItemListener* listener) { baseItemListeners.remove(listener); }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseItem)
};