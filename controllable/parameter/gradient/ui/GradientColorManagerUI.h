/*
  ==============================================================================

    GradientColorManagerUI.h
    Created: 11 Apr 2017 11:40:47am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class GradientColorManagerUI :
	public BaseManagerUI<GradientColorManager, GradientColor, GradientColorUI>,
	public ContainerAsyncListener,
	public Timer,
	public Thread

{
public:
	GradientColorManagerUI(GradientColorManager * manager);
	~GradientColorManagerUI();

	bool shouldRepaint;
	bool shouldUpdateImage;

	float viewStartPos;
	float viewEndPos;
	bool autoResetViewRangeOnLengthUpdate;

	Image viewImage;
	SpinLock imageLock;

	bool miniMode;

	Array<float> snapTimes;
	std::function<void(Array<float>*)> getSnapTimesFunc;
	
	void setViewRange(float start, float end);

	void setMiniMode(bool value);

	void paint(Graphics &g) override;

	void resized() override;

	void updateItemsVisibility() override;

    void addItemUIInternal(GradientColorUI * item) override;
    void removeItemUIInternal(GradientColorUI * item) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDoubleClick(const MouseEvent &e) override;
	void mouseDrag(const MouseEvent &e) override;

	void placeItemUI(GradientColorUI * tui);

	int getXForPos(float time);
	float getPosForX(int tx, bool offsetStart = true);

	bool isInView(GradientColorUI * tui);

	void newMessage(const ContainerAsyncEvent &e) override;

	void run() override;
	
	void timerCallback() override;
};