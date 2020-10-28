/*
  ==============================================================================

    GradientColorManager.h
    Created: 11 Apr 2017 11:40:41am
    Author:  Ben

  ==============================================================================
*/

#pragma once


class GradientColorComparator
{
public:
	int compareElements(GradientColor * t1, GradientColor * t2)
	{
		if (t1->position->floatValue() < t2->position->floatValue()) return -1;
		else if (t1->position->floatValue() > t2->position->floatValue()) return 1;
		return 0;
	}
};

class GradientColorManager :
	public BaseManager<GradientColor>
{
public:
	GradientColorManager(float maxPosition = 10, bool addDefaultColors = false, bool dedicatedSelectionManager = true);
	~GradientColorManager();

	ColorParameter * currentColor;
	SpinLock gradientLock;

	FloatParameter * position;
	FloatParameter * length;

	std::unique_ptr<InspectableSelectionManager> customSelectionManager;

	bool allowKeysOutside;
	
	void setLength(float val, bool stretch = false, bool stickToEnd = false);

	static GradientColorComparator comparator;

	Colour getColorForPosition(const float & time) const;
	
	//void rebuildGradient();
	
	GradientColor * addColorAt(float time, Colour color);
	GradientColor * getItemAt(float time, bool getNearestPreviousKeyIfNotFound = false) const;

	Array<GradientColor*> getItemsInTimespan(float startTime, float endTime);

	Array<UndoableAction*> getMoveKeysBy(float start, float offset);
	Array<UndoableAction*> getRemoveTimespan(float start, float end);


    void addItemInternal(GradientColor * item, var data) override;
	void removeItemInternal(GradientColor * item) override;

	Array<GradientColor*> addItemsFromClipboard(bool showWarning = false) override;

	
	void reorderItems() override;

	void onContainerParameterChanged(Parameter *) override;
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;

	void loadJSONDataInternal(var data) override;

	InspectableEditor* getEditor(bool isRoot) override;

	class GradientColorManagerListener
	{
	public:
		virtual ~GradientColorManagerListener() {}
		virtual void gradientUpdated() {}
	};

	ListenerList<GradientColorManagerListener> colorManagerListeners;
	void addColorManagerListener(GradientColorManagerListener* newListener) { colorManagerListeners.add(newListener); }
	void removeColorManagerListener(GradientColorManagerListener* listener) { colorManagerListeners.remove(listener); }


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientColorManager)
};