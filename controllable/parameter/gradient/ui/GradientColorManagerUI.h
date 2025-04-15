/*
  ==============================================================================

    GradientColorManagerUI.h
    Created: 11 Apr 2017 11:40:47am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class GradientColorManagerUI :
	public ManagerUI<GradientColorManager, GradientColor, GradientColorUI>,
	public ContainerAsyncListener,
	public UITimerTarget,
	public juce::Thread

{
public:
	GradientColorManagerUI(GradientColorManager * manager);
	~GradientColorManagerUI();

	bool shouldUpdateImage;

	float viewStartPos;
	float viewEndPos;
	bool autoResetViewRangeOnLengthUpdate;

	juce::Image viewImage;
	juce::SpinLock imageLock;

	bool miniMode;

	juce::Array<float> snapTimes;
	std::function<void(juce::Array<float>*)> getSnapTimesFunc;
	
	void setViewRange(float start, float end);

	void setMiniMode(bool value);

	void paint(juce::Graphics &g) override;
	void paintOverChildren(juce::Graphics& g) override;

	void resized() override;

	void updateItemsVisibility() override;

    void addItemUIInternal(GradientColorUI * item) override;
    void removeItemUIInternal(GradientColorUI * item) override;

	void mouseDown(const juce::MouseEvent& e) override;
	void mouseDoubleClick(const juce::MouseEvent &e) override;
	void mouseDrag(const juce::MouseEvent &e) override;

	void placeItemUI(GradientColorUI * tui);

	int getXForPos(float time);
	float getPosForX(int tx, bool offsetStart = true);

	bool isInView(GradientColorUI * tui);

	void newMessage(const ContainerAsyncEvent &e) override;

	void run() override;
	
	void handlePaintTimerInternal() override;
};