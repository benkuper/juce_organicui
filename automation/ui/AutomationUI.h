/*
  ==============================================================================

    AutomationUI.h
    Created: 11 Dec 2016 1:22:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class AutomationTimelineUIBase :
	public BaseManagerUI<AutomationBase,AutomationKeyBase,AutomationKeyTimelineUIBase>,
	public ContainerAsyncListener,
	public InspectableSelectionManager::Listener,
	public Thread,
	public Timer
{
public:
	AutomationTimelineUIBase(AutomationBase * _automation);
	virtual ~AutomationTimelineUIBase();
	
	//View optimisation, generate in thread a preview of the image
	enum ViewMode { EDIT, VIEW };
	ViewMode viewMode;
	bool autoSwitchMode;

	Image viewImage;
	bool shouldUpdateImage;
	SpinLock imageLock;

	float viewStartPos;
	float viewEndPos;

	int firstROIKey;
	int lastROIKey;

	bool autoResetViewRangeOnLengthUpdate;

	float currentPosition;
	float currentValue;

	bool fixedPosOrValueEnabled; //When using shift key and moving handles, keep either position or value

	AutomationKeyTimelineUIBase * currentUI;
	//std::unique_ptr<AutomationMultiKeyTransformer> transformer;

	bool shouldRepaint;

	void setCurrentPosition(const float &pos);
	void setCurrentValue(const float &val);

	void setViewMode(ViewMode mode);

	void setViewRange(float start, float end);
	void updateROI();

	void paint(Graphics &g) override;
	
	void resized() override;
	void placeKeyUI(AutomationKeyTimelineUIBase * kui, bool placePrevKUI = true);

	int getXForPos(float time);
	float getPosForX(int tx, bool offsetStart = true);

	int getYForValue(float value);
	float getValueForY(int ty);

	bool isInView(AutomationKeyTimelineUIBase * kui);


	AutomationKeyTimelineUIBase * getClosestKeyUIForPos(float pos, int start = - 1, int end = -1);

	void itemAddedAsync(AutomationKeyBase *) override;
	void itemsReorderedAsync() override;

	AutomationKeyTimelineUIBase * createUIForItem(AutomationKeyBase * item) override;
	
	virtual void addItemFromMouse(const MouseEvent& e) = 0;

	void addItemUIInternal(AutomationKeyTimelineUIBase *) override;
	void removeItemUIInternal(AutomationKeyTimelineUIBase *) override;

	void showMenuAndAddItem(bool, Point<int>) override {}; //no menu

	void mouseDown(const MouseEvent &e) override;
	void mouseDoubleClick(const MouseEvent &e) override;
	void mouseDrag(const MouseEvent &e) override;
	void mouseUp(const MouseEvent &e) override;
	bool keyPressed(const KeyPress &e) override;

	void newMessage(const ContainerAsyncEvent &e) override;

	void inspectablesSelectionChanged() override;
	void inspectableDestroyed(Inspectable *) override;

	void focusGained(FocusChangeType cause) override;
	void focusLost(FocusChangeType cause) override;

	//Generate image thread
	void run() override;
	virtual void drawPixelAtX(int tx) {}
	void timerCallback() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationTimelineUIBase)

};


template<class T>
class AutomationTimelineUI :
	public AutomationTimelineUIBase
{
public:
	AutomationTimelineUI(Automation<T> * automation);
	~AutomationTimelineUI() {}

	Automation<T> * typedManager;

	const Colour dimensionColors[3]{ RED_COLOR, GREEN_COLOR, BLUE_COLOR };

	virtual void addItemFromMouse(const MouseEvent &e) override;

	void drawPixelAtX(int tx) override;
};

template<class T>
AutomationTimelineUI<T>::AutomationTimelineUI(Automation<T> * automation) :
	AutomationTimelineUIBase(automation)
{
	typedManager = (Automation<T> *)manager;
}

template<>
void AutomationTimelineUI<float>::addItemFromMouse(const MouseEvent& e)
{
	typedManager->addItem(getPosForX(e.getPosition().x), getValueForY(e.getPosition().y));
}

template<>
void AutomationTimelineUI<Point<float>>::addItemFromMouse(const MouseEvent& e)
{
	typedManager->addItem(getPosForX(e.getPosition().x), Point<float>(getValueForY(e.getPosition().y),0));
}

template<class T>
void AutomationTimelineUI<T>::drawPixelAtX(int tx)
{
	Array<float> values = manager->getValuesForPosition(getPosForX(tx));

	for (int i = 0; i < manager->numDimensions; i++)
	{
		Colour c = manager->numDimensions == 1 ? Colours::white : dimensionColors[i];
		float y = (1 - values[i]) * (getHeight() - 1);
		int ty = (int)y;
		int maxDist = 1;
		for (int i = ty - maxDist; i <= ty + maxDist; i++)
		{
			if (i < 0 || i >= viewImage.getHeight()) continue;
			float alpha = jlimit<float>(0, 1, 1 - (abs(y - i) / maxDist));
			viewImage.setPixelAt(tx, i, c.withAlpha(alpha));
		}
	}
}

template<>
void AutomationTimelineUI<Vector3D<float>>::addItemFromMouse(const MouseEvent& e)
{
	typedManager->addItem(getPosForX(e.getPosition().x), Vector3D<float>(getValueForY(e.getPosition().y),0,0));
}
