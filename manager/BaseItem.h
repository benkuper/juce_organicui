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

class BaseItem :
	public EnablingControllableContainer
{
public:
	BaseItem(const juce::String& name = "", bool canBeDisabled = true, bool canHaveScript = false);
	virtual ~BaseItem();

	//UI - should move outside data class ? how to save/load if not there 
	BoolParameter* miniMode;
	FloatParameter* listUISize; //width or height in a list
	Point2DParameter* viewUIPosition; //position in a view
	Point2DParameter* viewUISize; //size in a view
	BoolParameter* isUILocked; //lock in UI
	ColorParameter* itemColor;

	bool useCustomArrowKeysBehaviour; // to have custom manipulation with arrow keys instead of selection prev/next

	bool canHaveScripts;
	bool userCanRemove;
	bool userCanDuplicate;
	bool askConfirmationBeforeRemove;
	bool isSavable;
	bool saveType;
	bool canBeReorderedInEditor;

	std::unique_ptr<ScriptManager> scriptManager;

	juce::String itemDataType;

	//UI moving X/Y
	juce::Point<float> movePositionReference;
	juce::Point<float> sizeReference;

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

	virtual void handleRemoveFromRemoteControl() override;

	void setMovePositionReference(bool setOtherSelectedItems = false);
	virtual void setMovePositionReferenceInternal();
	void movePosition(juce::Point<float> positionOffset, bool moveOtherSelectedItems = false);
	void scalePosition(juce::Point<float> positionOffset, bool moveOtherSelectedItems = false);
	virtual void setPosition(juce::Point<float> position);
	virtual juce::Point<float> getPosition();
	void addMoveToUndoManager(bool addOtherSelectedItems = false);
	virtual void addUndoableMoveAction(juce::Array<juce::UndoableAction *> &arrayToAdd);

	void setSizeReference(bool setOtherSelectedItems = false);
	virtual void setSizeReferenceInternal();
	void resizeItem(juce::Point<float> sizeOffset, bool resizeOtherSelectedItems = false);
	virtual void setItemSize(juce::Point<float> size);
	virtual juce::Point<float> getItemSize();

	void addResizeToUndoManager(bool addOtherSelectedItems = false);
	virtual void addUndoableResizeAction(juce::Array<juce::UndoableAction*>& arrayToAdd);



	//listeners
	virtual void onContainerParameterChanged(Parameter *) override;
	virtual void onContainerTriggerTriggered(Trigger *) override;
	virtual void onContainerParameterChangedInternal(Parameter *) {} //child classes override this function
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) {};

	//void itemAdded(Script* script) override;
	//void itemsAdded(juce::Array<Script*> scripts) override;

	void setHasCustomColor(bool value);

	juce::var getJSONData(bool includeNonOverriden = false) override;
	void loadJSONDataInternal(juce::var data) override;
	virtual void loadJSONDataItemInternal(juce::var data) {} //happens before loading scripts

	virtual void getRemoteControlDataInternal(juce::var& data) override;

	InspectableEditor * getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;
	virtual juce::String getTypeString() const { return "BaseItem"; };
	static juce::var getTypeStringFromScript(const juce::var::NativeFunctionArgs& a);
	juce::String getScriptTargetString() override;

	DECLARE_INSPECTACLE_SAFE_LISTENER(BaseItem, baseItem)

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseItem)
};