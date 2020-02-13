#pragma once


class Automation2DUI :
	public BaseManagerViewUI<Automation, AutomationKey, AutomationKey2DUI>,
	public ContainerAsyncListener,
	//public InspectableSelectionManager::Listener,
	//public Thread,
	public Timer
{
public:
	Automation2DUI(Automation* _automation);
	virtual ~Automation2DUI();

	float viewPosition;
	FloatParameter viewRange;
	std::unique_ptr<FloatParameterLabelUI> viewRangeUI;

	int firstROIKey;
	int lastROIKey;
	int numSteps;

	//AutomationKey2DUI* currentUI;

	bool shouldRepaint;

	
	//void setViewRange(float pos, float range);
	//void updateROI();

	void paint(Graphics& g) override;

	void updateViewUIPosition(AutomationKey2DUI * kui) override;

	//void resized() override;
	//void placeKeyUI(AutomationKeyUI* kui, bool placePrevKUI = true);

	//Point<int> getViewPosForPos(float time);
	////float getPosForViewPos(Point<int> viewPos);

	//int getYForValue(float value);
	//Array<int> getYForKey(AutomationKey* k);
	//float getValueForY(int ty, bool zeroIsBottom = true, bool relative = false);

	//bool isInView(AutomationKeyUI* kui);
	//void homeViewYRange();
	//void frameViewYRange();

	//AutomationKeyUI* getClosestKeyUIForPos(float pos, int start = -1, int end = -1);

	//void itemAddedAsync(AutomationKey*) override;
	//void itemsReorderedAsync() override;

	//AutomationKeyUI* createUIForItem(AutomationKey* item) override;

	//void addItemUIInternal(AutomationKeyUI*) override;
	//void removeItemUIInternal(AutomationKeyUI*) override;

	//void mouseDown(const MouseEvent& e) override;
	//void mouseDoubleClick(const MouseEvent& e) override;
	//void mouseDrag(const MouseEvent& e) override;
	//void mouseUp(const MouseEvent& e) override;
	//bool keyPressed(const KeyPress& e) override;

	void newMessage(const ContainerAsyncEvent& e) override;

	//void inspectablesSelectionChanged() override;
	//void inspectableDestroyed(Inspectable*) override;

	void timerCallback() override;
	

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Automation2DUI)

};