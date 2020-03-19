#pragma once


class Automation2DUI :
	public BaseManagerViewUI<Automation, AutomationKey, AutomationKey2DUI>,
	public ContainerAsyncListener,
	public AutomationKey2DUI::AsyncListener,
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

	bool shouldRepaint;

	Array<AutomationKey2DUI::Easing2DHandle*> handles;
	void paint(Graphics& g) override;

	void resizedInternalContent(Rectangle<int>& r) override;
	void updateViewUIPosition(AutomationKey2DUI * kui) override;

	void placeHandles();

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent &e) override;

	void updateHandlesForKey(AutomationKey2DUI* kui);

	void addItemUIInternal(AutomationKey2DUI* kui) override;
	void removeItemUIInternal(AutomationKey2DUI* kui) override;

	void newMessage(const ContainerAsyncEvent& e) override;
	void newMessage(const AutomationKey2DUI::Key2DUIEvent& e) override;

	void timerCallback() override;
	
private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Automation2DUI)

};