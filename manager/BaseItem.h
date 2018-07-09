/*
  ==============================================================================

    BaseItem.h
    Created: 28 Oct 2016 8:04:25pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef BASEITEM_H_INCLUDED
#define BASEITEM_H_INCLUDED


class ScriptManager;
class GenericControllableManager;

class BaseItem :
	public EnablingControllableContainer
{
public:
	BaseItem(const String &name = "", bool canBeDisabled = true, bool canHaveScript = false);
	virtual ~BaseItem();

	//UI - should move outside data class ? how to save/load if not there 
	BoolParameter * miniMode;
	FloatParameter * listUISize; //width or height in a list
	Point2DParameter * viewUIPosition; //position in a vieww
	Point2DParameter * viewUISize; //size in a view

	bool canHaveScripts;
	bool userCanRemove;
	bool askConfirmationBeforeRemove;
	bool isSavable;
	bool saveType;
	bool canBeReorderedInEditor;

	ScopedPointer<ScriptManager> scriptManager;

	String itemDataType;

	virtual void clearItem() {}

	virtual void duplicate();
	virtual void copy();
	virtual bool paste();

	virtual void moveBefore();
	virtual void moveAfter();

	void remove();

	virtual void onContainerParameterChanged(Parameter *) override;
	virtual void onContainerTriggerTriggered(Trigger *) override;
	virtual void onContainerParameterChangedInternal(Parameter *) {} //child classes override this function
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) {};

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	InspectableEditor * getEditor(bool isRoot) override;

	virtual String getTypeString() const { return "BaseItem"; };

	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void askForRemoveBaseItem(BaseItem *) {}
		virtual void askForDuplicateItem(BaseItem *) {}
		virtual void askForPaste() {}
		virtual void askForMoveBefore(BaseItem *) {}
		virtual void askForMoveAfter(BaseItem *) {}
	};


	ListenerList<Listener> baseItemListeners;
	void addBaseItemListener(Listener* newListener) { baseItemListeners.add(newListener); }
	void removeBaseItemListener(Listener* listener) { baseItemListeners.remove(listener); }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseItem)
};



#endif  // BASEITEM_H_INCLUDED