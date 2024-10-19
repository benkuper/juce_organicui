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
	juce::SpinLock gradientLock;

	FloatParameter * position;
	FloatParameter * length;

	std::unique_ptr<InspectableSelectionManager> customSelectionManager;

	bool allowKeysOutside;
	
	void setLength(float val, bool stretch = false, bool stickToEnd = false);
	void setAllowKeysOutside(bool value);
	void updateKeyRanges();

	static GradientColorComparator comparator;

	void updateCurrentColor();
	juce::Colour getColorForPosition(const float & time) const;
	
	//void rebuildGradient();
	
	GradientColor * addColorAt(float time, juce::Colour color);
	GradientColor * getItemAt(float time, bool getNearestPreviousKeyIfNotFound = false) const;

	juce::Array<GradientColor*> getItemsInTimespan(float startTime, float endTime);

	juce::Array<juce::UndoableAction*> getMoveKeysBy(float start, float offset);
	juce::Array<juce::UndoableAction*> getRemoveTimespan(float start, float end);


    void addItemInternal(GradientColor * item, juce::var data) override;
	void removeItemInternal(GradientColor * item) override;

	juce::Array<GradientColor*> addItemsFromClipboard(bool showWarning = false) override;

	
	void reorderItems() override;

	void onContainerParameterChanged(Parameter *) override;
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;

	static juce::var getColorAtPositionFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getKeyAtPositionFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getKeysBetweenFromScript(const juce::var::NativeFunctionArgs& a);

	void loadJSONDataInternal(juce::var data) override;

	InspectableEditor* getEditorInternal(bool isRoot, juce::Array<Inspectable*> inspectables = juce::Array<Inspectable*>()) override;

	class GradientColorManagerListener
	{
	public:
		virtual ~GradientColorManagerListener() {}
		virtual void gradientUpdated() {}
	};

	DECLARE_INSPECTACLE_SAFE_LISTENER(GradientColorManager, colorManager)


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientColorManager)
};