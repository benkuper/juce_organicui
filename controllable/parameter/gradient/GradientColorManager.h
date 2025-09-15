/*
  ==============================================================================

    GradientColorManager.h
    Created: 11 Apr 2017 11:40:41am
    Author:  Ben

  ==============================================================================
*/

#pragma once


class GradientColorManager :
	public Manager<GradientColor>
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

	void updateCurrentColor();
	juce::Colour getColorForPosition(const float & time);
	
	//void rebuildGradient();
	
	GradientColor * addColorAt(float time, juce::Colour color);
	GradientColor* getItemAt(float time, bool getNearestPreviousKeyIfNotFound = false);

	juce::Array<GradientColor*> getItemsInTimespan(float startTime, float endTime);

	juce::Array<juce::UndoableAction*> getMoveKeysBy(float start, float offset);
	juce::Array<juce::UndoableAction*> getRemoveTimespan(float start, float end);


    void addItemInternal(BaseItem * item, juce::var data) override;
	void removeItemInternal(BaseItem * item) override;

	juce::Array<BaseItem*> addItemsFromClipboard(bool showWarning = false) override;
	
	void onContainerParameterChanged(Parameter *) override;
	void onControllableFeedbackUpdate(ControllableContainer * cc, Controllable * c) override;

	static juce::var getColorAtPositionFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getKeyAtPositionFromScript(const juce::var::NativeFunctionArgs& a);
	static juce::var getKeysBetweenFromScript(const juce::var::NativeFunctionArgs& a);

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