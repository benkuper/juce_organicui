/*
  ==============================================================================

    AutomationUI.h
    Created: 11 Dec 2016 1:22:02pm
    Author:  Ben

  ==============================================================================
*/

#pragma once


class AutomationUI :
	public BaseManagerUI<Automation,AutomationKey,AutomationKeyUI>,
	public ContainerAsyncListener,
	public InspectableSelectionManager::Listener,
	public Thread,
	public Timer
{
public:
	AutomationUI(Automation * _automation);
	virtual ~AutomationUI();
	
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

	bool fixedPosOrValueEnabled; //When using shift key and moving handles, keep either position or value

	AutomationKeyUI * currentUI;
	//std::unique_ptr<AutomationMultiKeyTransformer> transformer;

	bool shouldRepaint;

	void setCurrentPosition(const float &pos);
	void setCurrentValue(const float &val);

	void setViewMode(ViewMode mode);

	void setViewRange(float start, float end);
	void updateROI();

	void paint(Graphics &g) override;
	
	void resized() override;
	void placeKeyUI(AutomationKeyUI * kui, bool placePrevKUI = true);

	int getXForPos(float time);
	float getPosForX(int tx, bool offsetStart = true);

	int getYForValue(float value);
	Array<int> getYForKey(AutomationKey * k);
	float getValueForY(int ty);

	bool isInView(AutomationKeyUI * kui);

	AutomationKeyUI * getClosestKeyUIForPos(float pos, int start = - 1, int end = -1);

	void itemAddedAsync(AutomationKey *) override;
	void itemsReorderedAsync() override;

	AutomationKeyUI * createUIForItem(AutomationKey * item) override;
	
	void addItemUIInternal(AutomationKeyUI *) override;
	void removeItemUIInternal(AutomationKeyUI *) override;

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

	void timerCallback() override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationUI)

};
